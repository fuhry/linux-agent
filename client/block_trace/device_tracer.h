#ifndef DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACER_H_
#define DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACER_H_

#include "block_trace/cpu_tracer.h"
#include "block_trace/trace_handler.h"

#include <linux/fs.h>
#include <linux/blktrace_api.h>

#include <memory>
#include <string>
#include <vector>

namespace datto_linux_client {

static const char DEBUG_FS_PATH[] = "/sys/kernel/debug";

// TODO Check for dropped traces
// TODO Check if switching to mmap is a good idea (performance)
// DeviceTracer is responsible for tracing the writes to a block device
// and handing off those traces to a TraceHandler instance
class DeviceTracer {
  // TODO Move these constants to another location, perhaps a config file
  static const int BLKTRACE_BUFFER_SIZE = 1024;
  static const int BLKTRACE_NUM_SUBBUFFERS = 10;
  static const int BLKTRACE_MASK = BLK_TC_QUEUE;

 public:
  // Starts a trace for the device specificed by block_dev_path
  // Traces will be sent to the TraceHandler instance
  DeviceTracer(const std::string &block_dev_path,
               std::shared_ptr<TraceHandler> handler);

  // Flush the trace buffers. This method returns once the buffers
  // have finished flushing and have been given to the TraceHandler
  virtual void FlushBuffers();

  virtual ~DeviceTracer();

  DeviceTracer(const DeviceTracer &) = delete;
  DeviceTracer& operator=(const DeviceTracer &) = delete;

 protected:
  // For creating stubs in unit testing
  DeviceTracer() : block_dev_fd_(-1) {}

 private:
  std::string BeginBlockTrace();
  void CleanupBlockTrace();
  std::string GetTracePath(int cpu_num);

  std::string block_dev_path_;
  std::string trace_name_;
  int block_dev_fd_;
  int num_cpus_;

  std::shared_ptr<TraceHandler> handler_;

  std::vector<std::unique_ptr<CpuTracer>> cpu_tracers_;
};

}

#endif //  DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACER_H_
