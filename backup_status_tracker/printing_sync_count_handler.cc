#include "backup_status_tracker/printing_sync_count_handler.h"

#include <stdio.h>

namespace datto_linux_client {

PrintingSyncCountHandler::PrintingSyncCountHandler(uint64_t bytes_total) {
  printf("%lu total bytes\n", bytes_total);
}

void PrintingSyncCountHandler::UpdateSyncedCount(uint64_t num_synced_a) {
  printf("%lu synced bytes\n", num_synced_a);
}

} // datto_linux_client
