#ifndef DATTO_CLIENT_BACKUP_BACKUP_H_
#define DATTO_CLIENT_BACKUP_BACKUP_H_

#include <memory>

#include "backup/backup_coordinator.h"
#include "block_device/block_device.h"
#include "block_device/mountable_block_device.h"
#include "device_synchronizer/device_synchronizer.h"
#include "unsynced_sector_manager/unsynced_sector_manager.h"

namespace datto_linux_client {

class Backup {
 public:
  explicit Backup(std::vector<DeviceSynchronizer> syncs_to_do);

  // This blocks until the backup is done. This won't throw an exception.
  virtual void DoBackup(std::shared_ptr<BackupEventHandler> event_handler);

  virtual ~Backup();

  Backup(const Backup &) = delete;
  Backup& operator=(const Backup &) = delete;

 protected:
  // Protected to allow for overriding in unit tests
  std::shared_ptr<BackupCoordinator> CreateBackupCoordinator();
};

} // datto_linux_client

#endif  // DATTO_CLIENT_BACKUP_BACKUP_H_
