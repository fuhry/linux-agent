#include "block_trace/trace_handler.h"

namespace datto_linux_client {

TraceHandler::TraceHandler(std::shared_ptr<TraceIntervalQueue> interval_queue,
                           std::mutex queue_mutex)
    : interval_queue_(interval_queue),
      queue_mutex_(queue_mutex) { }

void AddTrace(const struct blk_io_trace &trace_data) {
  std::lock_guard<std::mutex> lock(queue_mutex);
}

}
