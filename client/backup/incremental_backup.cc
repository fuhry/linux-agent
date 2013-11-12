#include "backup/incremental_backup.h"

namespace datto_linux_client {

IncrementalBackup::IncrementalBackup(
   std::shared_ptr<MountableBlockDevice> source_device,
   std::shared_ptr<UnsyncedSectorManager> source_unsynced_manager,
   std::shared_ptr<BlockDevice> destination_device,
   std::shared_ptr<BackupEventHandler> event_handler)
    : Backup(source_device,
             source_unsynced_manager,
             destination_device,
             event_handler) {}

void IncrementalBackup::Prepare(std::shared_ptr<CancellationToken> cancel_token) {
  // Start the tracer if it isn't going already
  if (!source_unsynced_manager_->IsTracing()) {
    source_unsynced_manager_->StartTracer();
  }
}

} // datto_linux_client
