#ifndef DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACE_H_
#define DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACE_H_

#include "block_trace/cpu_tracer.h"
#include "block_trace/interval_queue.h"
#include "block_trace/trace_handler.h"

#include <linux/fs.h>
#include <linux/blktrace_api.h>

#include <boost/icl/interval.hpp>
#include <boost/noncopyable.hpp>

#include <memory>
#include <string>
#include <vector>

namespace datto_linux_client {

static const std::string DEBUG_FS_PATH = "/sys/kernel/debug";

class DeviceTracer : private boost::noncopyable {
  static const int BLKTRACE_BUFFER_SIZE = 1024;
  static const int BLKTRACE_NUM_SUBBUFFERS = 10;
  static const int BLKTRACE_MASK = BLK_TC_QUEUE;

 public:
  // Don't pass a BlockDevice here as we want our own file descriptor
  // block_dev_path must be a real path to a block device (not a symlink)
  DeviceTracer(const std::string &block_dev_path,
               std::shared_ptr<TraceHandler> handler);

  void FlushBuffers();

  std::string block_dev_path();

  ~DeviceTracer();

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

#endif //  DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACE_H_
