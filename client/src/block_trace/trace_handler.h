#ifndef DATTO_CLIENT_BLOCK_TRACER_TRACE_HANDLER_H_
#define DATTO_CLIENT_BLOCK_TRACER_TRACE_HANDLER_H_

#include "linux/blktrace_api.h"
#include <boost/noncopyable.hpp>

#include "unsynced_sector_tracker/unsynced_sector_tracker.h"

namespace datto_linux_client {

class TraceHandler : private boost::noncopyable {
 static const int SECTOR_SIZE = 512;

 public:
  TraceHandler(std::shared_ptr<UnsyncedSectorTracker> tracker);
  void AddTrace(const struct blk_io_trace &trace_data);

 private:
  std::shared_ptr<UnsyncedSectorTracker> tracker_;

};

}

#endif //  DATTO_CLIENT_BLOCK_TRACER_TRACE_HANDLER_H_
