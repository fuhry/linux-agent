#include "backup/backup.h"
#include "backup/backup_exception.h"

#include <thread>

#include <glog/logging.h>

#include "device_synchronizer/device_synchronizer.h"

namespace datto_linux_client {

Backup::Backup(std::shared_ptr<MountableBlockDevice> source_device,
               std::shared_ptr<UnsyncedSectorManager> source_unsynced_manager,
               std::shared_ptr<BlockDevice> destination_device,
               std::shared_ptr<BackupEventHandler> event_handler)
    : source_device_(source_device),
      source_unsynced_manager_(source_unsynced_manager),
      destination_device_(destination_device),
      event_handler_(event_handler) {}

void Backup::DoBackup(std::shared_ptr<CancellationToken> cancel_token) {
  try {
    LOG(INFO) << "Preparing for backup";
    event_handler_->BackupPreparing();
    Prepare(cancel_token);

    if (cancel_token->ShouldCancel()) {
      LOG(INFO) << "Cancelled before copy";
      event_handler_->BackupCancelled();
      return;
    } 

    LOG(INFO) << "Copying data to destination device";
    event_handler_->BackupCopying();
    Copy(cancel_token);

    if (cancel_token->ShouldCancel()) {
      LOG(INFO) << "Cancelled during copy";
      // Mark all sectors marked as "synced" back to "unsynced"
      source_unsynced_manager_->store()->ResetUnsynced();
      event_handler_->BackupCancelled();
      return;
    } 

    // Backup succeeded so don't clear the synced blocks
    source_unsynced_manager_->store()->ClearSynced();
    event_handler_->BackupSucceeded();
  } catch (const std::runtime_error &e) {
    LOG(ERROR) << "Error during backup: " << e.what();
    event_handler_->BackupFailed(e.what());
  }
}

void Backup::Copy(std::shared_ptr<CancellationToken> cancel_token) {
  DeviceSynchronizer synchronizer(source_device_,
                                  source_unsynced_manager_,
                                  destination_device_,
                                  event_handler_);

  synchronizer.DoSync(cancel_token);
}

Backup::~Backup() { }

} // datto_linux_client
