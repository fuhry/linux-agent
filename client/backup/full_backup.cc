#include "backup/full_backup.h"

namespace datto_linux_client {

FullBackup::FullBackup(std::shared_ptr<MountableBlockDevice> source_device,
                       std::shared_ptr<UnsyncedSectorStore> source_sector_store,
                       std::shared_ptr<BlockDevice> destination_device,
                       std::shared_ptr<ReplyChannel> reply_channel)
    : Backup(source_device,
             source_sector_store,
             destination_device,
             reply_channel) {}

void FullBackup::Prepare() {
  // Add in-use sectors to the store
  auto in_use_set = source_device_->GetInUseSectors();

  for (const SectorInterval &interval : *in_use_set) {
    source_sector_store_->AddUnsyncedInterval(interval);
  }
}

void FullBackup::Cleanup() {
  // TODO: I think this can be empty
}

} // datto_linux_client
