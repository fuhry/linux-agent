#include "block_trace/device_tracer.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>

namespace datto_linux_client {

DeviceTracer::DeviceTracer(const std::string &block_dev_path)
    : block_dev_path_(block_dev_path),
      block_dev_fd_(-1) {

  if ((block_dev_fd_ =
        open(block_dev_path.c_str(), O_RDONLY | O_NONBLOCK)) == -1) {
    // TODO Improve this..
    throw "Unable to open block_dev_path";
  }

  interval_queue_(new IntervalQueue());

  PushEmptyInterval();

  num_cpus_ = get_nprocs();
  cpu_tracers_ = std::vector<std::unique_ptr<CpuTracer>>(num_cpus_);

  try {
    SetupBlockTrace();
    for (int i = 0; i < num_cpus_; ++i) {
      cpu_tracers_[i] = std::unique_ptr<CpuTracer>(
            new CpuTracer(block_dev_path_, i, nullptr));
    }
  } catch (...) {
    // If close fails we can't do much about it, so ignore the return value
    close(block_dev_fd_);
    throw;
  }
}

void DeviceTracer::PushEmptyInterval() {
  (interval_queue_)->push(std::shared_ptr<TraceInterval>(new TraceInterval()));
}

void DeviceTracer::SetupBlockTrace() { }

std::shared_ptr<const boost::icl::interval<uint64_t>>
DeviceTracer::PopInterval() {
  return nullptr;
}

DeviceTracer::~DeviceTracer() { }

}
