#ifndef DATTO_CLIENT_BLOCK_TRACER_CPU_TRACER_H_
#define DATTO_CLIENT_BLOCK_TRACER_CPU_TRACER_H_

#include <boost/icl/interval.hpp>

#include <atomic>
#include <memory>
#include <string>
#include <thread>

#include <stdint.h>

#include "block_trace/trace_handler.h"
#include "block_trace/block_trace_exception.h"

namespace datto_linux_client {

class CpuTracer {

 static const int POLL_DELAY_MILLIS = 100;

 public:
  CpuTracer(std::string &trace_path, int cpu_num,
            std::shared_ptr<TraceHandler> trace_handler); 

  // We don't always get instant notifications from the kernel that a trace
  // buffer has content. If a backup is about to occur, we need to flush it.
  void FlushBuffer();

  ~CpuTracer();

  CpuTracer(const CpuTracer &);
  CpuTracer& operator=(const CpuTracer &);

 private:
  void DoTrace();
  void LockOnCPU();
  void StopTrace();

  static void SkipNonseekableFD(int fd, int amount_to_skip);

  std::shared_ptr<TraceHandler> trace_handler_;
  std::thread trace_thread_;

  int trace_fd_;
  int cpu_num_;

  std::atomic_bool stop_trace_;
  std::atomic_bool flush_buffers_;
};

}

#endif //  DATTO_CLIENT_BLOCK_TRACER_CPU_TRACER_H_
