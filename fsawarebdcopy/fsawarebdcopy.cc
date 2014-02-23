#include <memory>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <glog/logging.h>

#include "backup/backup_manager.h"
#include "backup/fake_backup_coordinator.h"
#include "backup_status_tracker/printing_sync_count_handler.h"
#include "block_device/block_device_factory.h"
#include "device_synchronizer/device_synchronizer.h"

namespace {
using datto_linux_client::BackupCoordinator;
using datto_linux_client::BlockDevice;
using datto_linux_client::BlockDeviceFactory;
using datto_linux_client::DeviceSynchronizer;
using datto_linux_client::FakeBackupCoordinator;
using datto_linux_client::MountableBlockDevice;
using datto_linux_client::PrintingSyncCountHandler;
using datto_linux_client::SyncCountHandler;
using datto_linux_client::SectorInterval;
using datto_linux_client::UnsyncedSectorManager;
}

int main(int argc, char *argv[]) {
  // Setup logging

  // Only log to a memory location until a LogSink is written that doesn't
  // write when the file system is frozen!
  FLAGS_log_dir = "/dev/shm/";
  google::InitGoogleLogging(argv[0]);
  if (argc != 3) {
    printf("usage: %s source_device destination_device\n", argv[0]);
    exit(1);
  }
  std::string source(argv[1]);
  std::string destination(argv[2]);

  auto block_device_factory = std::make_shared<BlockDeviceFactory>();
  auto sector_manager = std::make_shared<UnsyncedSectorManager>();

  std::shared_ptr<MountableBlockDevice> source_device =
      block_device_factory->CreateMountableBlockDeviceFromPath(source);
  std::shared_ptr<BlockDevice> destination_device =
      std::make_shared<BlockDevice>(destination);

  auto source_store = sector_manager->GetStore(*source_device);

  auto in_use_set = source_device->GetInUseSectors();

  for (const SectorInterval &interval : *in_use_set) {
    source_store->AddUnsyncedInterval(interval);
  }

  uint64_t bytes_total = source_store->UnsyncedSectorCount() * 512;
  auto fake_coordinator = std::make_shared<FakeBackupCoordinator>(); 
  auto printing_counter =
      std::make_shared<PrintingSyncCountHandler>(bytes_total);

  DeviceSynchronizer device_synchronizer(source_device, sector_manager,
                                         destination_device);

  device_synchronizer.DoSync(fake_coordinator, printing_counter);

  printf("Sync of %s and %s complete\n", source.c_str(), destination.c_str());

  return 0;
}
