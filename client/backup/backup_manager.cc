#include "backup/backup_manager.h"

#include <thread>
#include <stdint.h>

#include "backup/backup_exception.h"
#include "backup/backup_event_handler.h"
#include "backup/full_backup.h"
#include "fs_parsing/ext_mountable_block_device.h"
#include "remote_block_device/nbd_block_device.h"

#include <glog/logging.h>

namespace datto_linux_client {

BackupManager::BackupManager()
    : in_progress_paths_(),
      cancel_tokens_mutex_(),
      cancel_tokens_(),
      managers_mutex_(),
      unsynced_managers_(),
      destructor_called_(false) {}

Reply BackupManager::StartBackup(const StartBackupRequest &start_request) {
  // TODO Add a meaningful reply
  Reply dummy;
  dummy.set_type(Reply::STRING);

  if (destructor_called_) {
    return dummy; // "Can't start during teardown"
  }
  std::lock_guard<std::mutex> m_lock(managers_mutex_);

  // 5 things are needed to start a backup
  //
  // Backup constructor requires four arguments:
  // 1. Source device
  // 2. Source unsynced sector manager
  // 3. Destination device
  // 4. Backup event handler
  //
  // DoBackup requires one argument:
  // 5. Cancellation token

  // TODO Check type of backup
  LOG(INFO) << "Starting full backup";
  std::string source_path = start_request.block_path();

  try {
    // Make sure the path isn't already being backed up
    in_progress_paths_.AddPathOrThrow(source_path);
  } catch (const BackupException &e) {
    return dummy; // "Backup already in progress"
  }

  try {
    // 1. Create the source ExtMountableBlockDevice
    std::shared_ptr<ExtMountableBlockDevice> source_device(
        new ExtMountableBlockDevice(source_path));

    // For a full, we can delete any already tracked sectors
    if (unsynced_managers_.count(source_path)) {
      unsynced_managers_.erase(source_path);
    }

    // 2. Create the source unsynced sector manager
    auto source_unsynced_manager_ =
      std::make_shared<UnsyncedSectorManager>(source_path);
    unsynced_managers_[source_path] = source_unsynced_manager_;

    // 3. Create the destination NbdBlockDevice
    std::string destination_host = start_request.destination_host();
    uint16_t destination_port = (uint16_t)start_request.destination_port();

    std::shared_ptr<NbdBlockDevice> destination_device(
        new NbdBlockDevice(destination_host, destination_port));

    // Start the tracer so we catch blocks that change during the backup
    source_unsynced_manager_->StartTracer();

    // 4. Create an event handler which is notified and acts on progress change
    std::shared_ptr<BackupEventHandler> event_handler =
        std::make_shared<BackupEventHandler>("dummy-job-guid");

    // Create the actual backup object
    std::unique_ptr<Backup> backup(new FullBackup(source_device,
                                                  source_unsynced_manager_,
                                                  destination_device,
                                                  event_handler));
    // TODO: 5. Create the cancellation token
    auto cancel_token = std::make_shared<CancellationToken>();

    // Start the backup in a detached thread
    //
    // We can capture by reference on in_progress_paths_ because
    // BackupManager's destructor guarantees that in_progress_paths_ persists
    // until all backup threads are complete
    std::thread backup_thread([=, &backup, &in_progress_paths_]() {
      std::unique_ptr<Backup> thread_local_backup = std::move(backup);
      thread_local_backup->DoBackup(cancel_token);
      in_progress_paths_.RemovePath(source_path);
    });

    backup_thread.detach();

    while (backup) {
      std::this_thread::yield();
    }
  } catch (const std::runtime_error &e) {
    // TODO Make this more clear:
    // We can only do this here (without worrying about a race with the
    // RemovePath call in backup_thread) because everything after
    // backup_thread's creation won't throw
    in_progress_paths_.RemovePath(source_path);
    LOG(ERROR) << e.what();
    return dummy; // e.what();
  }

  return dummy; // success
}

Reply BackupManager::StopBackup(const StopBackupRequest &stop_request) {
  std::string source_path = stop_request.block_path();

  std::lock_guard<std::mutex> c_lock(cancel_tokens_mutex_);

  // The weak_ptr will be default constructed if it isn't in the map
  auto token = cancel_tokens_[source_path].lock();

  if (token) {
    token->Cancel();
  }

  Reply dummy;
  dummy.set_type(Reply::STRING);
  return dummy;
}

BackupManager::~BackupManager() {
  destructor_called_ = true;
  while (in_progress_paths_.Count()) {
    std::this_thread::yield();
  }
}


} // datto_linux_client
