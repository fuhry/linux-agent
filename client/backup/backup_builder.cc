#include "backup/backup_builder.h"

#include "backup/backup_exception.h"
#include "device_synchronizer/device_synchronizer.h"

#include <glog/logging.h>

namespace datto_linux_client {

std::shared_ptr<Backup> BackupBuilder::CreateBackup(
    const std::vector<DevicePair> &device_pairs,
    const std::shared_ptr<BackupCoordinator> &coordinator,
    bool is_full) {
  std::vector<std::shared_ptr<DeviceSynchronizerInterface>> syncs_to_do;

  for (auto device_pair : device_pairs) {
    syncs_to_do.push_back(CreateDeviceSynchronizer(device_pair, is_full));
  }

  return std::make_shared<Backup>(syncs_to_do, coordinator);
}

std::shared_ptr<DeviceSynchronizerInterface>
BackupBuilder::CreateDeviceSynchronizer(const DevicePair &device_pair,
                                        bool is_full) {
  auto source_device =
      block_device_factory_->CreateMountableBlockDevice(
          device_pair.block_device_uuid());

  std::string host = device_pair.destination_host();
  uint16_t port = (uint16_t)device_pair.destination_port();

  auto remote_device =
      block_device_factory_->CreateRemoteBlockDevice(host, port);

  if (!sector_manager_->IsTracing(*source_device)) {
    if (is_full) {
      sector_manager_->StartTracer(*source_device);
    } else {
      LOG(ERROR) << source_device->path() << " has no trace data."
                 << " Must do a full.";
      throw BackupException("No trace data exists for source");
    }
  }

  if (is_full) {
    auto store = sector_manager_->GetStore(*source_device);
    store->ClearAll();
    for (auto interval : *(source_device->GetInUseSectors())) {
      store->AddUnsyncedInterval(interval);
    }
  }

  return std::make_shared<DeviceSynchronizer>(source_device,
                                              sector_manager_,
                                              remote_device);
}

} // datto_linux_client
