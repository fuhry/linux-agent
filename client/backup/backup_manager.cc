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
    : backup_runner_tracker_(),
      managers_mutex_(),
      unsynced_managers_() {}

// TODO This shouldn't throw, it should only return error replys
Reply BackupManager::StartBackup(const StartBackupRequest &start_request) {
  std::lock_guard<std::mutex> m_lock(managers_mutex_);

  // TODO Check type of backup

  LOG(INFO) << "Starting full backup";

  std::string source_path = start_request.block_path();

  // For a full, we can delete any already tracked sectors
  if (unsynced_managers_.count(source_path)) {
    unsynced_managers_.erase(source_path);
  }

  // Create the unsynced sector manager
  auto source_unsynced_manager_ =
      std::make_shared<UnsyncedSectorManager>(source_path);
  unsynced_managers_[source_path] = source_unsynced_manager_;

  // Create the ExtMountableBlockDevice
  std::shared_ptr<ExtMountableBlockDevice> source_device(
      new ExtMountableBlockDevice(source_path));

  // Create the NbdBlockDevice
  std::string destination_host = start_request.destination_host();
  uint16_t destination_port = (uint16_t)start_request.destination_port();

  std::shared_ptr<NbdBlockDevice> destination_device(
      new NbdBlockDevice(destination_host, destination_port));

  // Start the tracer so we catch blocks that change during the backup
  source_unsynced_manager_->StartTracer();

  // Create an event handler which is notified and acts on progress changes
  std::shared_ptr<BackupEventHandler> event_handler =
      std::make_shared<BackupEventHandler>("dummy-job-guid");

  // Create the backup object
  std::unique_ptr<FullBackup> backup(new FullBackup(source_device,
                                                    source_unsynced_manager_,
                                                    destination_device,
                                                    event_handler));

  // Start the backup
  // TODO cancel token
  backup_runner_tracker_.StartRunner(std::move(backup), nullptr);

  // TODO Add a meaningful reply
  Reply dummy;
  dummy.set_type(Reply::STRING);
  return dummy;
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

// We depend on the BackupRunnerTracker destructor to block until all backups
// finish
BackupManager::~BackupManager() { }


} // datto_linux_client
