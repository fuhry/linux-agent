#ifndef DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_INTERFACE_H_
#define DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_INTERFACE_H_

#include <memory>

#include "backup/backup_coordinator.h"
#include "backup_status_tracker/sync_count_handler.h"
#include "block_device/block_device.h"
#include "block_device/mountable_block_device.h"
#include "unsynced_sector_manager/unsynced_sector_manager.h"

namespace datto_linux_client {

// Existance of this class allows for easier mocking in unit tests
class DeviceSynchronizerInterface {
 public:
  // Precondition: source_device must be both traced and mounted
  virtual void DoSync(std::shared_ptr<BackupCoordinator> coordinator,
                      std::shared_ptr<SyncCountHandler> event_handler) = 0;

  virtual std::shared_ptr<const MountableBlockDevice>
  source_device() const = 0;

  virtual std::shared_ptr<const UnsyncedSectorManager>
  source_unsynced_manager() const = 0;

  virtual std::shared_ptr<const BlockDevice> destination_device() const = 0;

  virtual ~DeviceSynchronizerInterface() {}
};
}

#endif //  DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_INTERFACE_H_
