#ifndef DATTO_CLIENT_BACKUP_INCREMENTAL_BACKUP_H_
#define DATTO_CLIENT_BACKUP_INCREMENTAL_BACKUP_H_

#include <string>

#include "backup/backup.h"
#include "block_device/block_device.h"

namespace datto_linux_client {

class IncrementalBackup : public Backup {
 public:
  IncrementalBackup(
      std::shared_ptr<MountableBlockDevice> source_device,
      std::shared_ptr<UnsyncedSectorManager> source_unsynced_manager,
      std::shared_ptr<BlockDevice> destination_device,
      std::shared_ptr<BackupEventHandler> event_handler);

  void Prepare(std::shared_ptr<CancellationToken> cancel_token);
};

} // datto_linux_client

#endif  // DATTO_CLIENT_BACKUP_INCREMENTAL_BACKUP_H_
