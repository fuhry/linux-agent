#ifndef DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACE_H_
#define DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACE_H_

#include "block_trace/cpu_tracer.h"
#include "block_trace/trace_handler.h"

#include "linux/blktrace_api.h"

#include <boost/icl/interval.hpp>
#include <boost/noncopyable.hpp>

#include <memory>
#include <string>
#include <vector>

#include <stdint.h>

#include "block_trace/interval_queue.h"

namespace datto_linux_client {

static const std::string DEBUG_FS_PATH = "/sys/kernel/debug";

class DeviceTracer : private boost::noncopyable {
 public:
  // Don't pass a BlockDevice here as we want our own file descriptor
  DeviceTracer(const std::string &block_dev_path, std::shared_ptr<TraceHandler> handler);

  std::string block_dev_path();

  ~DeviceTracer();

 private:
  void SetupBlockTrace();
  void TeardownBlockTrace();
  std::string GetTracePath(int cpu_num, std::string block_dev_name);

  std::string block_dev_path_;
  int block_dev_fd_;
  int num_cpus_;

  std::shared_ptr<TraceHandler> handler_;

  std::vector<std::unique_ptr<CpuTracer>> cpu_tracers_;

};

}

#endif //  DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACE_H_
