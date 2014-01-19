#ifndef DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_H_
#define DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_H_

#include <memory>

#include "block_device/block_device.h"
#include "block_device/mountable_block_device.h"
#include "device_synchronizer/device_synchronizer_interface.h"
#include "unsynced_sector_manager/unsynced_sector_manager.h"

namespace datto_linux_client {

class DeviceSynchronizer : public DeviceSynchronizerInterface {
 public:
  DeviceSynchronizer(
      std::shared_ptr<MountableBlockDevice> source_device,
      std::shared_ptr<UnsyncedSectorManager> source_unsynced_manager,
      std::shared_ptr<BlockDevice> destination_device);

  // Precondition: source_device must be both traced and mounted
  void DoSync(std::shared_ptr<BackupCoordinator> coordinator,
              std::shared_ptr<BackupEventHandler> event_handler);

  ~DeviceSynchronizer();

 private:
  std::shared_ptr<MountableBlockDevice> source_device_;
  std::shared_ptr<UnsyncedSectorManager> source_unsynced_manager_;
  std::shared_ptr<BlockDevice> destination_device_;
};
}

#endif //  DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_H_
