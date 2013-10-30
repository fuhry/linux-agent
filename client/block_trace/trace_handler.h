#ifndef DATTO_CLIENT_BLOCK_TRACER_TRACE_HANDLER_H_
#define DATTO_CLIENT_BLOCK_TRACER_TRACE_HANDLER_H_

#include <linux/blktrace_api.h>
#include <boost/noncopyable.hpp>
#include <memory>

#include "unsynced_sector_store/unsynced_sector_store.h"

namespace datto_linux_client {

class TraceHandler : private boost::noncopyable {
 public:
  // TODO: This needs to be a calculated value
  static const int SECTOR_SIZE = 512;

  explicit TraceHandler(std::shared_ptr<UnsyncedSectorStore> store);
  virtual void AddTrace(const struct blk_io_trace &trace_data);
  virtual ~TraceHandler() { }

 protected:
  // For derived classes (mainly for testing) that don't want to
  // use UnsyncedSectorStore
  TraceHandler() { }

 private:
  std::shared_ptr<UnsyncedSectorStore> store_;

};

}

#endif //  DATTO_CLIENT_BLOCK_TRACER_TRACE_HANDLER_H_
