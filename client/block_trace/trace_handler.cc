#include "block_trace/trace_handler.h"
#include <glog/logging.h>

namespace datto_linux_client {

TraceHandler::TraceHandler(std::shared_ptr<UnsyncedSectorStore> store)
    : store_(store) { }

void TraceHandler::AddTrace(const struct blk_io_trace &trace_data) {
  if (trace_data.action & BLK_TC_ACT(BLK_TC_WRITE)) {
    VLOG(1) << "Adding trace with action 0x"
              << std::hex << trace_data.action << std::dec;

    uint64_t sectors_written = trace_data.bytes / SECTOR_SIZE;
    VLOG(2) << sectors_written << " sectors";

    uint64_t sector = trace_data.sector;
    SectorInterval interval(sector, sector + sectors_written);
    (store_)->AddUnsyncedInterval(interval);
  } else {
    VLOG(2) << "Discarding trace with action 0x"
              << std::hex << trace_data.action << std::dec;
  }
}

}
