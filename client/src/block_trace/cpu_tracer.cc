#include "cpu_tracer.h"

namespace datto_linux_client {

CpuTracer::CpuTracer(std::string &block_dev_path, int cpu_num,
                     std::shared_ptr<IntervalQueue> trace_queue)
    : trace_queue_(trace_queue) {

}

CpuTracer::~CpuTracer() { }

}
