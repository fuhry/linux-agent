#include "backup/full_backup.h"

namespace datto_linux_client {

FullBackup::FullBackup(
   std::shared_ptr<MountableBlockDevice> source_device,
   std::shared_ptr<UnsyncedSectorManager> source_unsynced_manager,
   std::shared_ptr<BlockDevice> destination_device,
   std::shared_ptr<BackupEventHandler> event_handler)
    : Backup(source_device,
             source_unsynced_manager,
             destination_device,
             event_handler) {}

void FullBackup::Prepare(std::shared_ptr<CancellationToken> cancel_token) {
  // Add in-use sectors to the store
  auto in_use_set = source_device_->GetInUseSectors();

  // TODO Check if this is slow enough to make it worth optimizing
  for (const SectorInterval &interval : *in_use_set) {
    source_unsynced_manager_->store()->AddUnsyncedInterval(interval);
  }
}

} // datto_linux_client
