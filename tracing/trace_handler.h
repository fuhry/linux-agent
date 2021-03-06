#ifndef DATTO_CLIENT_BLOCK_TRACE_TRACE_HANDLER_H_
#define DATTO_CLIENT_BLOCK_TRACE_TRACE_HANDLER_H_

#include <linux/blktrace_api.h>
#include <memory>

#include "unsynced_sector_manager/unsynced_sector_store.h"

namespace datto_linux_client {

// TODO: Can the store be unique_ptr?
class TraceHandler {
 public:
  // TODO: This needs to be a calculated value
  static const int SECTOR_SIZE = 512;

  explicit TraceHandler(std::shared_ptr<UnsyncedSectorStore> store);
  virtual void AddTrace(const struct blk_io_trace &trace_data);
  virtual ~TraceHandler() { }

  TraceHandler(const TraceHandler &) = delete;
  TraceHandler& operator=(const TraceHandler &) = delete;
 protected:
  // For derived classes (mainly for testing) that don't want to
  // use UnsyncedSectorStore
  TraceHandler() { }

 private:
  std::shared_ptr<UnsyncedSectorStore> store_;
};

}

#endif //  DATTO_CLIENT_BLOCK_TRACE_TRACE_HANDLER_H_
