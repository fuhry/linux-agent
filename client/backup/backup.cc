#include "backup/backup.h"

#include <thread>

#include <glog/logging.h>

#include "device_synchronizer/device_synchronizer.h"

namespace datto_linux_client {

Backup::Backup(std::shared_ptr<MountableBlockDevice> source_device,
               std::shared_ptr<UnsyncedSectorManager> sector_manager,
               std::shared_ptr<BlockDevice> destination_device,
               std::shared_ptr<ReplyChannel> reply_channel)
    : source_device_(source_device),
      sector_manager_(sector_manager),
      destination_device_(destination_device),
      reply_channel_(reply_channel),
      status_(BackupStatus::NOT_STARTED),
      do_stop_(false) {}


void Backup::DoBackup() {
  try {
    LOG(INFO) << "Preparing for backup";
    status_ = BackupStatus::PREPARING;
    Prepare();

    if (do_stop_) {
      LOG(INFO) << "Stopping";
    } else {
      LOG(INFO) << "Copying data to destination device";
      status_ = BackupStatus::COPYING;
      Copy();
    }

    LOG(INFO) << "Cleaning up after backup";
    status_ = BackupStatus::CLEANING_UP;
    Cleanup();

    status_ = BackupStatus::FINISHED;
  } catch (const std::runtime_error &e) {
    LOG(ERROR) << e.what();
    status_ = BackupStatus::FAILED;
    throw;
  }
}

void Backup::Copy() {
  auto source_store = sector_manager_->GetStore(source_device_->path());

  DeviceSynchronizer synchronizer(source_device_,
                                  source_store,
                                  destination_device_,
                                  reply_channel_);

  synchronizer.StartSync();

  while (!synchronizer.done() && !do_stop_) {
    // TODO Some sort of accounting/progress verification here
    // TODO Freeze FS when almost done
    std::this_thread::yield();
  }
  // TODO Unfreeze FS when done
}

void Backup::Stop() {
  do_stop_ = true;
}

} // datto_linux_client
