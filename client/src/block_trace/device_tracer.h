#ifndef DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACE_H_
#define DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACE_H_

#include "block_trace/cpu_tracer.h"

#include <boost/icl/interval.hpp>
#include <boost/noncopyable.hpp>

#include <memory>
#include <string>
#include <vector>

#include <stdint.h>

namespace datto_linux_client {

static const std::string DEBUG_FS_PATH = "/sys/kernel/debug";

class DeviceTracer : private boost::noncopyable {
 public:
  DeviceTracer(const std::string &block_dev_path);

  void PushEmptyInterval();
  std::shared_ptr<const boost::icl::interval<uint64_t>> PopInterval();

  std::string block_dev_path();

  ~DeviceTracer();

 private:
  void SetupBlockTrace();

  std::string block_dev_path_;
  int block_dev_fd_;
  int num_cpus_;

  std::vector<std::unique_ptr<CpuTracer>> cpu_tracers_;

};

}

#endif //  DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACE_H_
