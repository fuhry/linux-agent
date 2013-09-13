#ifndef DATTO_CLIENT_BLOCK_TRACER_TRACE_HANDLER_H_
#define DATTO_CLIENT_BLOCK_TRACER_TRACE_HANDLER_H_

#include "linux/blktrace_api.h"

namespace datto_linux_client {

class TraceHandler : private boost::noncopyable {
 public:
  TraceHandler(std::shared_ptr<TraceIntervalQueue> interval_queue
               std::mutex queue_mutex_);

  void AddTrace(const struct blk_io_trace &trace_data);

 private:
  std::shared_ptr<TraceIntervalQueue> interval_queue_;

}

}

#endif //  DATTO_CLIENT_BLOCK_TRACER_TRACE_HANDLER_H_
