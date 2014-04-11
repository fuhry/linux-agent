#ifndef DATTO_CLIENT_BACKUP_BACKUP_BUILDER_H_
#define DATTO_CLIENT_BACKUP_BACKUP_BUILDER_H_

#include <memory>
#include <vector>

#include "backup/backup.h"
#include "block_device/block_device_factory.h"
#include "device_synchronizer/device_synchronizer_interface.h"
#include "unsynced_sector_manager/unsynced_sector_manager.h"

#include "vector.pb.h"

namespace datto_linux_client {

class BackupBuilder {
 public:
  BackupBuilder(std::shared_ptr<BlockDeviceFactory> block_device_factory,
                std::shared_ptr<UnsyncedSectorManager> sector_manager)
      : block_device_factory_(block_device_factory),
        sector_manager_(sector_manager) {}
      
  virtual ~BackupBuilder() {}

  // This blocks until the backup is done. This won't throw an exception.
  virtual std::shared_ptr<Backup> CreateBackup(
      const std::vector<Vector> &vectors,
      const std::shared_ptr<BackupCoordinator> &coordinator,
      bool is_full);

  BackupBuilder(const BackupBuilder &) = delete;
  BackupBuilder& operator=(const BackupBuilder &) = delete;

 protected:
  // For unit testing
  virtual std::shared_ptr<DeviceSynchronizerInterface>
  CreateDeviceSynchronizer(const Vector &vector, bool is_full);

  BackupBuilder() {}

 private:
  std::shared_ptr<BlockDeviceFactory> block_device_factory_;
  std::shared_ptr<UnsyncedSectorManager> sector_manager_;
};

} // datto_linux_client

#endif  // DATTO_CLIENT_BACKUP_BACKUP_BUILDER_H_
