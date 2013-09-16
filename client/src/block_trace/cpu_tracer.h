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
  CpuTracer(std::string &block_dev_name, int cpu_num,
            TraceHandler &trace_handler); 
  ~CpuTracer();

 private:
  TraceHandler trace_handler_;
};

}

#endif //  DATTO_CLIENT_BLOCK_TRACER_CPU_TRACER_H_
