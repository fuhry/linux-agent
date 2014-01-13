#include "block_trace/device_tracer.h"

#include "block_trace/block_trace_exception.h"

#include <fcntl.h>
#include <glog/logging.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>

namespace datto_linux_client {

DeviceTracer::DeviceTracer(const std::string &block_dev_path,
                           std::shared_ptr<TraceHandler> handler)
    : handler_(handler) {

  if ((block_dev_fd_ = open(block_dev_path.c_str(),
                            O_RDONLY | O_NONBLOCK)) < 0) {
    PLOG(ERROR) << "Unable to open " << block_dev_path;
    throw BlockTraceException("Unable to open device for tracing");
  }

  num_cpus_ = get_nprocs_conf();
  DLOG(INFO) << "num_cpus_: " << num_cpus_;
  cpu_tracers_ = std::vector<std::unique_ptr<CpuTracer>>(num_cpus_);

  try {
    trace_name_ = BeginBlockTrace();
    DLOG(INFO) << "trace_name_: " << trace_name_;

    for (int i = 0; i < num_cpus_; ++i) {
      std::string trace_path = GetTracePath(i);
      DLOG(INFO) << "trace_path: " << trace_path;

      cpu_tracers_[i] = std::unique_ptr<CpuTracer>(
                          new CpuTracer(trace_path, i, handler_));
    }
  } catch (...) {

    try {
      CleanupBlockTrace();
    } catch (const std::exception &e) {
      // Log, but don't rethrow as we are in a try/catch already
      // and the outer exception is more important
      LOG(ERROR) << "Exception in DeviceTracer constructor during cleanup: "
                 << e.what();
    } catch (...) {
      LOG(ERROR) << "Non-exception thrown in DeviceTracer constructor "
                 << "during cleanup";
    }

    // If close fails we can't do much about it, so ignore the return value
    close(block_dev_fd_);
    throw;
  }
}

void DeviceTracer::FlushBuffers() {
  for (auto &cpu_tracer : cpu_tracers_) {
    cpu_tracer->FlushBuffer();
  }
}

std::string DeviceTracer::BeginBlockTrace() {
  struct blk_user_trace_setup blktrace_setup = {};

  blktrace_setup.buf_size = BLKTRACE_BUFFER_SIZE;
  blktrace_setup.buf_nr = BLKTRACE_NUM_SUBBUFFERS;
  blktrace_setup.act_mask = BLKTRACE_MASK;

  if (ioctl(block_dev_fd_, BLKTRACESETUP, &blktrace_setup) < 0) {
    PLOG(ERROR) << "BLKTRACESETUP with fd: " << block_dev_fd_;
    throw BlockTraceException("BLKTRACESETUP");
  }

  if (ioctl(block_dev_fd_, BLKTRACESTART) < 0) {
    PLOG(ERROR) << "BLKTRACESTART with fd: " << block_dev_fd_;
    throw BlockTraceException("BLKTRACESTART");
  }

  return std::string(blktrace_setup.name);

}

void DeviceTracer::CleanupBlockTrace() {
  DLOG(INFO) << "In CleanupBlockTrace";

  if (block_dev_fd_ != -1) {
    if (ioctl(block_dev_fd_, BLKTRACESTOP) < 0) {
      PLOG(ERROR) << "BLKTRACESTOP";
      throw BlockTraceException("Unable to stop blocktrace");
    }

    if (ioctl(block_dev_fd_, BLKTRACETEARDOWN) < 0) {
      PLOG(ERROR) << "BLKTRACETEARDOWN";
      throw BlockTraceException("Unable to teardown blocktrace");
    }
  }

}

std::string DeviceTracer::GetTracePath(int cpu_num) {

  // In general this path will be something like
  // /sys/kernel/debug/block/sda1/trace1
  std::string trace_path = std::string(DEBUG_FS_PATH) + "/block/" +
                           trace_name_ + "/trace" + std::to_string(cpu_num);

  struct stat stat_buf;
  if (stat(trace_path.c_str(), &stat_buf) == -1) {
    PLOG(ERROR) << "stat on trace_path";
    throw BlockTraceException("Error during stat");
  }

  if (!S_ISREG(stat_buf.st_mode)) {
    PLOG(ERROR) << "stat on trace_path";
    throw BlockTraceException("Bad trace path: " + trace_path);
  }

  return trace_path;
}

DeviceTracer::~DeviceTracer() {
  try {
    DLOG(INFO) << "Clearing CPU tracers";
    cpu_tracers_.clear();

    CleanupBlockTrace();
    close(block_dev_fd_);
  } catch (const std::exception &e) {
    LOG(ERROR) << "Exception in DeviceTracer destructor: " << e.what();
  }
}

}
