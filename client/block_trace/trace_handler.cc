#include "block_trace/trace_handler.h"

namespace datto_linux_client {

TraceHandler::TraceHandler(std::shared_ptr<UnsyncedSectorTracker> tracker)
    : tracker_(tracker) { }

void TraceHandler::AddTrace(const struct blk_io_trace &trace_data) {
  uint64_t sectors_written = trace_data.bytes / SECTOR_SIZE;
  uint64_t sector = trace_data.sector;
  SectorInterval interval(sector, sector + sectors_written);
  (tracker_)->AddUnsyncedInterval(interval);
}

}
