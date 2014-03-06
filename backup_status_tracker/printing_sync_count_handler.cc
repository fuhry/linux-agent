#include "backup_status_tracker/printing_sync_count_handler.h"

#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <inttypes.h>

namespace datto_linux_client {

PrintingSyncCountHandler::PrintingSyncCountHandler(uint64_t bytes_total) {
  printf("%" PRIu64 " total bytes\n", bytes_total);
}

void PrintingSyncCountHandler::UpdateSyncedCount(uint64_t num_synced_a) {
  printf("%" PRIu64 " synced bytes\n", num_synced_a);
}

} // datto_linux_client
