#ifndef DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_H_
#define DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_H_

#include "device_synchronizer/device_synchronizer_interface.h"

namespace datto_linux_client {

// DeviceSynchronizer is responsible to get one block device
// synchronized with another block device. While the destination
// can be any block device, in the main program it will always be
// some type of RemoteBlockDevice
class DeviceSynchronizer : public DeviceSynchronizerInterface {
 public:
  // @source_device: The block device with the filesystem that will be copied
  //                 to the destination device.
  // @source_manager: The data manager that knows what sectors need to be
  //                  copied from one location to another.
  // @destination_device: The block device that will have the @source_device
  //                      copied onto it.
  DeviceSynchronizer(
      std::shared_ptr<MountableBlockDevice> source_device,
      std::shared_ptr<UnsyncedSectorManager> sector_manager,
      std::shared_ptr<BlockDevice> destination_device);

  // Precondition: source_device must be both traced and mounted
  //
  // @coordinator: Provides communication methods with the other
  //               DeviceSynchronizers in the Backup
  // @count_handler: Used to update the progress of the sychronization
  void DoSync(std::shared_ptr<BackupCoordinator> coordinator,
              std::shared_ptr<SyncCountHandler> count_handler);

  std::shared_ptr<const MountableBlockDevice> source_device() const {
    return source_device_;
  }

  std::shared_ptr<const UnsyncedSectorManager> sector_manager() const {
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
