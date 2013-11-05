#include "backup/backup.h"

#include <thread>

#include <glog/logging.h>

#include "device_synchronizer/device_synchronizer.h"

namespace datto_linux_client {

Backup::Backup(std::shared_ptr<MountableBlockDevice> source_device,
               std::shared_ptr<UnsyncedSectorManager> source_unsynced_manager,
               std::shared_ptr<BlockDevice> destination_device,
               std::shared_ptr<ReplyChannel> reply_channel)
    : source_device_(source_device),
      source_unsynced_manager_(source_unsynced_manager),
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
    LOG(ERROR) << "Error during backup: " << e.what();
    status_ = BackupStatus::FAILED;
    throw;
  }
}

void Backup::Copy() {
  DeviceSynchronizer synchronizer(source_device_,
                                  source_unsynced_manager_,
                                  destination_device_,
                                  reply_channel_);

  synchronizer.StartSync();

  while (!synchronizer.done() && !do_stop_) {
    // TODO Some sort of accounting/progress verification here
    std::this_thread::yield();
  }
}

void Backup::Stop() {
  do_stop_ = true;
}

} // datto_linux_client
