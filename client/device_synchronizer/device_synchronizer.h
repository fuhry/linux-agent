#ifndef DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_H_
#define DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_H_

#include <memory>

#include "cancellation/cancellation_token.h"
#include "block_device/block_device.h"
#include "block_device/mountable_block_device.h"
#include "backup_event_tracker/backup_event_handler.h"
#include "unsynced_sector_manager/unsynced_sector_manager.h"

namespace datto_linux_client {

class DeviceSynchronizer {
 public:
  DeviceSynchronizer(
      std::shared_ptr<MountableBlockDevice> source_device,
      std::shared_ptr<UnsyncedSectorManager> source_unsynced_manager,
      std::shared_ptr<BlockDevice> destination_device,
      std::shared_ptr<BackupEventHandler> event_handler);

  // Precondition to running this is source_device must be both traced and
  // mounted
  void DoSync(std::shared_ptr<CancellationToken> cancel_token);

 ~DeviceSynchronizer();

 private:
  std::shared_ptr<MountableBlockDevice> source_device_;
  std::shared_ptr<UnsyncedSectorManager> source_unsynced_manager_;
  std::shared_ptr<BlockDevice> destination_device_;
  std::shared_ptr<BackupEventHandler> event_handler_;
};
}

#endif //  DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_H_
