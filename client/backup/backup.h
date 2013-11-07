#ifndef DATTO_CLIENT_BACKUP_BACKUP_H_
#define DATTO_CLIENT_BACKUP_BACKUP_H_

#include <memory>

#include "backup/backup_event_handler.h"
#include "block_device/block_device.h"
#include "block_device/mountable_block_device.h"
#include "cancellation/cancellation_token.h"
#include "unsynced_sector_manager/unsynced_sector_manager.h"

namespace datto_linux_client {

class Backup {
 public:
  Backup(std::shared_ptr<MountableBlockDevice> source_device,
         std::shared_ptr<UnsyncedSectorManager> source_unsynced_manager,
         std::shared_ptr<BlockDevice> destination_device,
         std::shared_ptr<BackupEventHandler> event_handler);

  // This blocks until the backup is done. This won't throw an exception.
  void DoBackup(std::shared_ptr<CancellationToken> cancel_token);

  virtual ~Backup();

  Backup(const Backup &) = delete;
  Backup& operator=(const Backup &) = delete;
 protected:
  virtual void Prepare(std::shared_ptr<CancellationToken> cancel_token) = 0;
  virtual void Copy(std::shared_ptr<CancellationToken> cancel_token);

  std::shared_ptr<MountableBlockDevice> source_device_;
  std::shared_ptr<UnsyncedSectorManager> source_unsynced_manager_;
  std::shared_ptr<BlockDevice> destination_device_;
  std::shared_ptr<BackupEventHandler> event_handler_;
};

} // datto_linux_client

#endif  // DATTO_CLIENT_BACKUP_BACKUP_H_
