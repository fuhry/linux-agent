#include "cpu_tracer.h"
#include <algorithm>
#include <fcntl.h>
#include <glog/logging.h>
#include <linux/blktrace_api.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace datto_linux_client {

CpuTracer::CpuTracer(std::string &trace_path,
                     std::shared_ptr<TraceHandler> trace_handler)
    : trace_handler_(trace_handler),
      stop_trace_(false) {

  trace_fd_ = open(trace_path.c_str(), O_RDONLY | O_NONBLOCK);

  if (trace_fd_ == -1) {
    // TODO
    throw "Unable to open trace_path";
  }

  trace_thread_ = std::thread(&CpuTracer::DoTrace, this);

}

// As this is the initial function of a thread, this method must not throw an
// exception or the entire program will go down
void CpuTracer::DoTrace() {
  struct pollfd pfd;
  pfd.fd = trace_fd_;
  pfd.events = POLLIN;

  int poll_val = -1;
  struct blk_io_trace trace;

  while (!stop_trace_ &&
      ((poll_val = poll(&pfd, 1, POLL_DELAY_MILLIS)) >= 0)) {
    if (poll_val == 0 && !flush_buffers_) {
      // Poll timed out and we aren't forcing a flush
      continue;
    }

    int read_bytes = read(trace_fd_, &trace, sizeof(trace));
    if (read_bytes == -1) {
      PLOG(ERROR) << "Error while reading trace file descriptor";
      break;
    } else if (read_bytes == 0) {
      // TODO Signal buffers are flushed
      flush_buffers_ = false;
      continue;
    }

    if (trace.magic != (BLK_IO_TRACE_MAGIC | BLK_IO_TRACE_VERSION)) {
      PLOG(ERROR) << "Bad magic number in trace";
      break;
    }

    try {
      (trace_handler_)->AddTrace(trace);
    } catch (const std::exception &e) {
      PLOG(ERROR) << "Exception while adding trace" << e.what();
      break;
    }
    VLOG(2) << "Action is 0x"
      << std::hex << trace.action << std::dec;

    if (trace.pdu_len) {
      // Skip over any trailing data
      // TODO Log to see if this ever actually happens
      VLOG(2) << "Skipping trailing data for action 0x"
                 << std::hex << trace.action << std::dec;
      try {
        SkipNonseekableFD(trace_fd_, trace.pdu_len);
      } catch (...) {
        LOG(ERROR) << "Error while seeking reading unused payload from trace";
        break;
      }
    }
  }

  if (poll_val < 0) {
    PLOG(ERROR) << "poll";
  }

  // Don't worry about the return value as we aren't writing anything
  close(trace_fd_);
}

void CpuTracer::FlushBuffer() {
  flush_buffers_ = true;
  // TODO Condition variable
}

void CpuTracer::StopTrace() {
  stop_trace_ = true;
  if (trace_thread_.joinable()) {
    trace_thread_.join();
  }
  // TODO Condition variable
}

void CpuTracer::SkipNonseekableFD(int fd, int amount_to_skip) {
  // Can't use sendfile() here because the output isn't a socket
  // Seems to be no way to avoid copying unneeded data to userspace

  char buf[512];
  int read_bytes;

  while (amount_to_skip > 0) {
    read_bytes = read(fd, &buf, std::min(amount_to_skip, 512));
    if (read_bytes == -1) {
      // TODO: Use errno
      throw "Unable to read from fd";
    }
    amount_to_skip -= read_bytes;
  }

}

CpuTracer::~CpuTracer() {
  StopTrace();
}

}
