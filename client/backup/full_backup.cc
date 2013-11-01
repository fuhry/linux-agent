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

void FullBackup::Prepare() {
  // Add in-use sectors to the store
  auto in_use_set = source_device_->GetInUseSectors();
  auto source_store = sector_manager_->GetStore(source_device_->path());

  for (const SectorInterval &interval : *in_use_set) {
    source_store->AddUnsyncedInterval(interval);
  }

  // Start tracing so we catch writes that occur during the backup
  sector_manager_->StartTracer(source_device_->path());
}

void FullBackup::Cleanup() {
  // TODO: I think this can be empty
}

} // datto_linux_client
