#include "backup/backup_manager.h"

#include <thread>
#include <stdint.h>
#include <uuid/uuid.h>

#include "backup/backup_exception.h"
#include "backup_event_tracker/backup_event_tracker.h"
#include "backup_event_tracker/backup_event_handler.h"
#include "backup/full_backup.h"
#include "backup/incremental_backup.h"
#include "fs_parsing/ext_mountable_block_device.h"
#include "remote_block_device/nbd_block_device.h"

#include <glog/logging.h>

namespace {

std::string make_uuid() {
  uuid_t uuid;
  uuid_generate_time(uuid);
  char uuid_c_str[36];
  uuid_unparse(uuid, uuid_c_str);
  return std::string(uuid_c_str);
}

}

namespace datto_linux_client {

BackupManager::BackupManager()
    : in_progress_paths_(),
      backup_event_tracker_(),
      cancel_tokens_mutex_(),
      cancel_tokens_(),
      managers_mutex_(),
      unsynced_managers_(),
      destructor_called_(false) {}

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
//
// managers_mutex should be held before calling this
std::unique_ptr<Backup> CreateBackup(std::string source_path,
                                     std::string destination_host,
                                     uint16_t destination_port,
                                     int backup_type) {

  // 1. Create the source ExtMountableBlockDevice
  std::shared_ptr<ExtMountableBlockDevice> source_device(
      new ExtMountableBlockDevice(source_path));

  // 2. Get (and create if needed) the source unsynced sector manager
  if (!unsynced_managers_.count(source_path)) {
    unsynced_managers_[source_path] = 
      std::make_shared<UnsyncedSectorManager>(source_path);
  }
  auto source_unsynced_manager_ = unsynced_managers_[source_path];

  // 3. Create the destination NbdBlockDevice
  std::string destination_host = start_request.destination_host();
  uint16_t destination_port = (uint16_t)start_request.destination_port();

  std::shared_ptr<NbdBlockDevice> destination_device(
      new NbdBlockDevice(destination_host, destination_port));

  // 4. Create an event handler which is notified and acts on progress change
  auto event_handler = backup_event_tracker_.CreateEventHandler(uuid);

  // 5. Create the CancellationToken
  auto cancel_token = std::make_shared<CancellationToken>();
  {
    std::lock_guard<std::mutex> c_lock(cancel_tokens_mutex_);
    // TODO: 5. Create the cancellation token
    cancel_tokens_[uuid] = cancel_token;
  }

  // Create the actual backup object
  std::unique_ptr<Backup> backup;

  switch (start_request.type()) {
    case StartBackupRequest::FULL_BACKUP:
      backup = std::move(std::unique_ptr<Backup>(
          new FullBackup(source_device,
                         source_unsynced_manager_,
                         destination_device,
                         std::move(event_handler))));
      break;
    case StartBackupRequest::INCREMENTAL_BACKUP:
      backup = std::move(std::unique_ptr<Backup>(
          new IncrementalBackup(source_device,
                                source_unsynced_manager_,
                                destination_device,
                                std::move(event_handler))));
      break;
    default:
      throw BackupException("Not implemented");
  }

  return backup;
}

Reply BackupManager::StartBackup(const StartBackupRequest &start_request) {
  std::string uuid = make_uuid();

  Reply reply;
  reply.set_type(Reply::START_BACKUP);
  reply.mutable_start_backup_reply()->set_job_uuid(uuid);

  // Message will be populated on error
  Reply error_reply;
  error_reply.set_type(Reply::ERROR);

  if (destructor_called_) {
    error_reply.set_short_error("Can't start during teardown");
    return error_reply;
  }
  std::lock_guard<std::mutex> m_lock(managers_mutex_);

  // TODO Check type of backup
  LOG(INFO) << "Starting full backup";
  DevicePair device_pair = start_request.device_pair();

  std::string source_path = device_pair.block_path();
  std::string destination_host = device_pair.destination_host();
  uint16_t destination_port = (uint16_t)device_pair.destination_port();

  try {
    // Make sure the path isn't already being backed up
    in_progress_paths_.AddPathOrThrow(source_path);
  } catch (const BackupException &e) {
    error_reply.set_short_error(
        "Backup already in progress for " + source_path);
    return error_reply;
  }

  try {
    std::unique_ptr<Backup> backup = CreateBackup(source_path,
                                                  destination_host,
                                                  destination_port,
                                                  start_backup_request.type);
  } catch (const std::runtime_error &e) {
    in_progress_paths_.RemovePath(source_path);
    LOG(ERROR) << e.what();
    error_reply.set_short_error(
        "Problem while creating the backup for " + source_path);
    error_reply.set_long_error(e.what());
    return error_reply;
  }

  // Start the backup in a detached thread
  //
  // We can capture by reference on in_progress_paths_ because
  // BackupManager's destructor guarantees that in_progress_paths_ persists
  // until all backup threads are complete
  std::thread backup_thread([=, &backup]() {
    std::unique_ptr<Backup> thread_local_backup = std::move(backup);
    thread_local_backup->DoBackup(cancel_token);
    this->in_progress_paths_.RemovePath(source_path);
  });

  backup_thread.detach();

  // Wait for the backup_thread to take ownership of the backup
  while (backup) {
    std::this_thread::yield();
  }

  return reply; // success
}

Reply BackupManager::StopBackup(const StopBackupRequest &stop_request) {
  // Find the cancellation token if it exists and use it to cancel
  {
    std::lock_guard<std::mutex> c_lock(cancel_tokens_mutex_);

    std::shared_ptr<CancellationToken> cancel_token =
      cancel_tokens_[stop_request.job_uuid()].lock();

    if (cancel_token) {
      cancel_token->Cancel();
    }
  }

  Reply reply;
  reply.set_type(Reply::STOP_BACKUP);
  return reply;
}

Reply BackupManager::BackupStatus(const BackupStatusRequest &status_request) {
  Reply reply;
  try {
    auto status_reply =
      backup_event_tracker_.GetReply(status_request.job_uuid());
    // status_reply will be nullptr if it didn't exist
    if (status_reply) {
      reply.set_type(Reply::BACKUP_STATUS);
      *reply.mutable_backup_status_reply() = *status_reply;
    }
  } catch (const std::runtime_error &e) {
    LOG(ERROR) << "Error getting status: " << e.what();
  }

  if (!reply.has_type()) {
    reply.set_type(Reply::ERROR);
    reply.mutable_error_reply()->set_short_error("Job didn't exist");
    reply.mutable_error_reply()->set_long_error(
        "Couldn't find job uuid: " + status_request.job_uuid());
  }

  return reply;
}

BackupManager::~BackupManager() {
  destructor_called_ = true;

  // Cancel all backups
  {
    std::lock_guard<std::mutex> c_lock(cancel_tokens_mutex_);
    for (auto cancel_pair_ : cancel_tokens_) {
      auto cancel_token = cancel_pair_.second.lock();
      if (cancel_token) {
        cancel_token->Cancel();
      }
    }
  }

  // Wait for all backups to finish
  while (in_progress_paths_.Count()) {
    std::this_thread::yield();
  }
}

} // datto_linux_client
