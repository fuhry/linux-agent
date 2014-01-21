#ifndef DATTO_CLIENT_BACKUP_STATUS_TRACKER_SYNC_COUNT_HANDLER_H_
#define DATTO_CLIENT_BACKUP_STATUS_TRACKER_SYNC_COUNT_HANDLER_H_

#include <memory>
#include <unistd.h>

#include "block_device_status.pb.h"

namespace datto_linux_client {

class SyncCountHandler {
 public:
  explicit SyncCountHandler(std::shared_ptr<BlockDeviceStatus> status);
  virtual ~SyncCountHandler() {}

  // num_synced should be the total synced
  virtual void UpdateSyncedCount(uint64_t num_synced);
  // num_unsynced should be the total synced
  virtual void UpdateUnsyncedCount(uint64_t num_unsynced);

  SyncCountHandler(const SyncCountHandler &) = delete;
  SyncCountHandler& operator=(const SyncCountHandler &) = delete;
 protected:
  // For unit testing
  SyncCountHandler() {}
};

} // datto_linux_client

#endif  // DATTO_CLIENT_BACKUP_STATUS_TRACKER_SYNC_COUNT_HANDLER_H_
