#ifndef DATTO_CLIENT_BACKUP_FULL_BACKUP_H_
#define DATTO_CLIENT_BACKUP_FULL_BACKUP_H

#include <string>

#include "backup/backup.h"
#include "block_device/block_device.h"

namespace datto_linux_client {

class FullBackup : public Backup {
 public:
  FullBackup(std::shared_ptr<MountableBlockDevice> source_device,
             std::shared_ptr<UnsyncedSectorManager> sector_manager,
             std::shared_ptr<BlockDevice> destination_device,
             std::shared_ptr<ReplyChannel> reply_channel);

  void Prepare();
  void Cleanup();
};

} // datto_linux_client

#endif  // DATTO_CLIENT_BACKUP_FULL_BACKUP_H_
