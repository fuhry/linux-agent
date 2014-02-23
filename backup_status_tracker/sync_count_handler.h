#ifndef DATTO_CLIENT_BACKUP_STATUS_TRACKER_SYNC_COUNT_HANDLER_H_
#define DATTO_CLIENT_BACKUP_STATUS_TRACKER_SYNC_COUNT_HANDLER_H_

#include <memory>
#include <mutex>
#include <unistd.h>

#include "block_device_status.pb.h"

namespace datto_linux_client {

class SyncCountHandler {
 public:
  SyncCountHandler(BlockDeviceStatus *block_device_status,
                   std::shared_ptr<std::mutex> to_lock_mutex);
  virtual ~SyncCountHandler() {}

  // num_synced should be the total synced
  virtual void UpdateSyncedCount(uint64_t num_synced);
  // num_unsynced should be the total synced
  virtual void UpdateUnsyncedCount(uint64_t num_unsynced);

  SyncCountHandler(const SyncCountHandler &) = delete;
  SyncCountHandler& operator=(const SyncCountHandler &) = delete;
 protected:
  // For unit testing and subclasses
  SyncCountHandler() {}
 private:
  BlockDeviceStatus *block_device_status_;
  std::shared_ptr<std::mutex> to_lock_mutex_;
};

} // datto_linux_client

#endif  // DATTO_CLIENT_BACKUP_STATUS_TRACKER_SYNC_COUNT_HANDLER_H_
