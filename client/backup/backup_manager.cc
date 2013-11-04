#include "backup/backup_manager.h"

#include <thread>
#include <stdint.h>

#include "backup/backup_exception.h"
#include "backup/full_backup.h"
#include "fs_parsing/ext_mountable_block_device.h"
#include "remote_block_device/nbd_block_device.h"

#include <glog/logging.h>

namespace datto_linux_client {

BackupManager::BackupManager()
    : in_progress_mutex_(),
      in_progress_backups_(),
      managers_mutex_(),
      unsynced_managers_() {}

void BackupManager::StartFullBackup(
    const StartBackupRequest &start_request,
    std::shared_ptr<ReplyChannel> reply_channel) {

  std::lock_guard<std::mutex> ip_lock(in_progress_mutex_);
  std::lock_guard<std::mutex> m_lock(managers_mutex_);

  LOG(INFO) << "Starting full backup";

  std::string source_path = start_request.block_path();

  // Make sure there isn't a backup in progress
  if (in_progress_backups_.count(source_path)) {
    throw BackupException("Backup is already in progress");
  }

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

  // Create the backup object
  std::unique_ptr<FullBackup> backup(new FullBackup(source_device,
                                                    source_unsynced_manager_,
                                                    destination_device,
                                                    reply_channel));

  // Track it in our progress map
  in_progress_backups_[source_path] = std::move(backup);

  // Start it
  in_progress_backups_[source_path]->DoBackup();
}

void BackupManager::StartIncrementalBackup(
    const StartBackupRequest &start_request) {
  std::lock_guard<std::mutex> ip_lock(in_progress_mutex_);
  std::lock_guard<std::mutex> m_lock(managers_mutex_);

  throw std::runtime_error("Not implemented!");
}

void BackupManager::StopBackup(const StopBackupRequest &stop_request) {
  std::lock_guard<std::mutex> ip_lock(in_progress_mutex_);

  std::string source_path = stop_request.block_path();
  std::unique_ptr<Backup> backup =
      std::move(in_progress_backups_.at(source_path));

  backup->Stop();
  // backup destructor will be called here
}

BackupManager::~BackupManager() {
  // TODO: destructors should handle this but double check
}


} // datto_linux_client
