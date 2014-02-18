#include "backup_status_tracker/sync_count_handler.h"

namespace datto_linux_client {

SyncCountHandler::SyncCountHandler(BlockDeviceStatus *block_device_status,
                                   std::shared_ptr<std::mutex> to_lock_mutex)
    : block_device_status_(block_device_status),
      to_lock_mutex_(to_lock_mutex) {
  std::lock_guard<std::mutex> lock(*to_lock_mutex_);
  block_device_status_->set_bytes_transferred(0);
}

void SyncCountHandler::UpdateSyncedCount(uint64_t num_synced) {
  std::lock_guard<std::mutex> lock(*to_lock_mutex_);
  block_device_status_->set_bytes_transferred(num_synced);
}
void SyncCountHandler::UpdateUnsyncedCount(uint64_t num_unsynced) {
  std::lock_guard<std::mutex> lock(*to_lock_mutex_);
  block_device_status_->set_bytes_unsynced(num_unsynced);
}

} // datto_linux_client
