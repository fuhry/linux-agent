#include "backup/full_backup.h"

namespace datto_linux_client {

FullBackup::FullBackup(std::shared_ptr<MountableBlockDevice> source_device,
                       std::shared_ptr<UnsyncedSectorManager> sector_manager,
                       std::shared_ptr<BlockDevice> destination_device,
                       std::shared_ptr<ReplyChannel> reply_channel)
    : Backup(source_device,
             sector_manager,
             destination_device,
             reply_channel) {}

FullBackup::Prepare() {
  // Add in-use sectors to the store
  auto source_in_use = source_device->GetInUseSectors();
  sector_manager_->GetStore()->AddUnsyncedInterval(*source_in_use);

  // Start tracing so we catch writes that occur during the backup
  sector_manager_->StartTracer(source_device_->path());
}

FullBackup::Cleanup() {
  // TODO: I think this can be empty
}

} // datto_linux_client

#endif  // DATTO_CLIENT_BACKUP_H_
