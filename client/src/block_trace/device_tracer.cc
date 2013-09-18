#include "block_trace/device_tracer.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>

namespace datto_linux_client {

DeviceTracer::DeviceTracer(const std::string &block_dev_path,
                           std::shared_ptr<TraceHandler> handler)
    : block_dev_path_(block_dev_path),
      handler_(handler) {

  if ((block_dev_fd_ =
        open(block_dev_path.c_str(), O_RDONLY | O_NONBLOCK)) == -1) {
    // TODO Improve this..
    throw "Unable to open block_dev_path";
  }

  num_cpus_ = get_nprocs_conf();
  cpu_tracers_ = std::vector<std::unique_ptr<CpuTracer>>(num_cpus_);

  try {
    BeginBlockTrace();
    for (int i = 0; i < num_cpus_; ++i) {
      std::string trace_path = GetTracePath(i, /* TODO */ "");

      cpu_tracers_[i] = std::unique_ptr<CpuTracer>(
            new CpuTracer(trace_path, handler_));
    }
  } catch (...) {
    // If close fails we can't do much about it, so ignore the return value
    close(block_dev_fd_);
    throw;
  }
}

void DeviceTracer::BeginBlockTrace() {
  struct blk_user_trace_setup blktrace_setup = {};

  blktrace_setup.buf_size = BLKTRACE_BUFFER_SIZE;
  blktrace_setup.buf_nr = BLKTRACE_NUM_SUBBUFFERS;
  blktrace_setup.act_mask = BLKTRACE_MASK;

  if (ioctl(block_dev_fd_, BLKTRACESETUP, &blktrace_setup) < 0) {
    // TODO
    throw "Unable to setup blocktrace";
  }

  if (ioctl(block_dev_fd_, BLKTRACESTART) < 0) {
    // TODO
    throw "Unable to start blocktrace";
  }

}

void DeviceTracer::CleanupBlockTrace() {
  // TODO Should we throw here or just be silent?
  // Probably throw and let the destructor catch..?

  if (ioctl(block_dev_fd_, BLKTRACESTOP) < 0) {
    // TODO
    throw "Unable to stop blocktrace";
  }

  if (ioctl(block_dev_fd_, BLKTRACETEARDOWN) < 0) {
    // TODO
    throw "Unable to teardown blocktrace";
  }

}

std::string DeviceTracer::GetTracePath(int cpu_num,
                                       std::string block_dev_name) {
  std::string trace_path = DEBUG_FS_PATH + "/block/" + block_dev_name +
                           "/trace" + std::to_string(cpu_num);

  struct stat stat_buf;
  if ((stat(trace_path.c_str(), &stat_buf)) == -1) {
    // TODO
    throw "Error during stat";
  }

  if (!S_ISREG(stat_buf.st_mode)) {
    // Unable to generate trace path
    throw "Bad trace path: " + trace_path;
  }

  return trace_path;
}

DeviceTracer::~DeviceTracer() {
  try {
    CleanupBlockTrace();
  } catch (...) {
    // TODO: Log
  }
}

}
