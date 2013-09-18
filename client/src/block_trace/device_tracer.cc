#include "block_trace/device_tracer.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>

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

  num_cpus_ = get_nprocs();
  cpu_tracers_ = std::vector<std::unique_ptr<CpuTracer>>(num_cpus_);

  try {
    SetupBlockTrace();
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

void DeviceTracer::SetupBlockTrace() { }

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

DeviceTracer::~DeviceTracer() { }

}
