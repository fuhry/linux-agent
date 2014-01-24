#ifndef DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_H_
#define DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_H_

#include "device_synchronizer/device_synchronizer_interface.h"

namespace datto_linux_client {

class DeviceSynchronizer : public DeviceSynchronizerInterface {
 public:
  DeviceSynchronizer(
      std::shared_ptr<MountableBlockDevice> source_device,
      std::shared_ptr<UnsyncedSectorManager> sector_manager,
      std::shared_ptr<BlockDevice> destination_device);

  // Precondition: source_device must be both traced and mounted
  void DoSync(std::shared_ptr<BackupCoordinator> coordinator,
              std::shared_ptr<SyncCountHandler> count_handler);

  std::shared_ptr<const MountableBlockDevice> source_device() const {
    return source_device_;
  }

  std::shared_ptr<const UnsyncedSectorManager>
  sector_manager() const {
    return sector_manager_;
  }

  std::shared_ptr<const BlockDevice> destination_device() const {
    return destination_device_;
  }

  ~DeviceSynchronizer();

 private:
  std::shared_ptr<MountableBlockDevice> source_device_;
  std::shared_ptr<UnsyncedSectorManager> sector_manager_;
  std::shared_ptr<BlockDevice> destination_device_;
};
}

#endif //  DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_H_
