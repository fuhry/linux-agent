#ifndef DATTO_CLIENT_BLOCK_TRACER_CPU_TRACER_H_
#define DATTO_CLIENT_BLOCK_TRACER_CPU_TRACER_H_

#include <boost/icl/interval.hpp>
#include <boost/noncopyable.hpp>

#include <memory>
#include <queue>
#include <string>

#include <stdint.h>

#include "block_trace/interval_queue.h"

namespace datto_linux_client {

class CpuTracer : private boost::noncopyable {
 public:
  CpuTracer(std::string &block_dev_path, int cpu_num, std::shared_ptr<IntervalQueue> trace_queue);
  ~CpuTracer();

 private:
  std::shared_ptr<IntervalQueue> trace_queue_;
};

}

#endif //  DATTO_CLIENT_BLOCK_TRACER_CPU_TRACER_H_
