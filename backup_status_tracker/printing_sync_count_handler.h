#ifndef DATTO_CLIENT_BACKUP_STATUS_TRACKER_PRINTING_SYNC_COUNT_HANDLER_H_
#define DATTO_CLIENT_BACKUP_STATUS_TRACKER_PRINTING_SYNC_COUNT_HANDLER_H_

#include "backup_status_tracker/sync_count_handler.h"

namespace datto_linux_client {
class PrintingSyncCountHandler : public SyncCountHandler {
 public:
  explicit PrintingSyncCountHandler(uint64_t bytes_total);
  ~PrintingSyncCountHandler() {}

  // num_synced should be the total synced
  virtual void UpdateSyncedCount(uint64_t num_synced_a);
  // num_unsynced should be the total synced
  virtual void UpdateUnsyncedCount(uint64_t num_unsynced_a) {}

  PrintingSyncCountHandler(const SyncCountHandler &) = delete;
  PrintingSyncCountHandler& operator=(const SyncCountHandler &) = delete;

 private:
  uint64_t num_synced;
  uint64_t num_unsynced;
};
} // datto_linux_client

#endif  // DATTO_CLIENT_BACKUP_STATUS_TRACKER_PRINTING_SYNC_COUNT_HANDLER_H
