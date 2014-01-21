#ifndef DATTO_CLIENT_BACKUP_STATUS_TRACKER_SYNC_COUNT_HANDLER_H_
#define DATTO_CLIENT_BACKUP_STATUS_TRACKER_SYNC_COUNT_HANDLER_H_

#include <memory>
#include <mutex>
#include <string>
#include <unistd.h>

#include "block_device_status.pb.h"

namespace datto_linux_client {

class SyncCountHandler {
 public:
  explicit SyncCountHandler(std::shared_ptr<BlockDeviceStatus> status);
  virtual ~SyncCountHandler() {}

  // num_synced should be the total synced
  void UpdateSyncedCount(uint64_t num_synced);
  // num_unsynced should be the total synced
  void UpdateUnsyncedCount(uint64_t num_unsynced);

  SyncCountHandler(const SyncCountHandler &) = delete;
  SyncCountHandler& operator=(const SyncCountHandler &) = delete;

 private:
  const std::string job_uuid_;
  std::shared_ptr<std::mutex> to_lock_mutex_;
  std::shared_ptr<BackupStatusReply> reply_;
};

} // datto_linux_client

#endif  // DATTO_CLIENT_BACKUP_STATUS_TRACKER_SYNC_COUNT_HANDLER_H_
