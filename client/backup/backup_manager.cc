#include "backup/backup_manager.h"

#include <glog/logging.h>
#include <stdint.h>
#include <string>
#include <uuid/uuid.h>

#include "backup/backup_exception.h"

namespace {
std::string make_uuid() {
  uuid_t uuid;
  uuid_generate_time(uuid);
  char uuid_c_str[36];
  uuid_unparse(uuid, uuid_c_str);
  return std::string(uuid_c_str);
}
} // anonymous namespace

namespace datto_linux_client {

std::string BackupManager::StartBackup(
    const StartBackupRequest &start_request) {

  if (destructor_called_) {
    throw BackupException("Unable to start backup, program is tearing down");
  }

  std::lock_guard<std::mutex> lock(start_backup_mutex_);

  int num_device_pairs = start_request.device_pairs_size();

  std::vector<DevicePair> device_pairs(start_request.device_pairs().begin(),
                                       start_request.device_pairs().end());

  auto coordinator = std::make_shared<BackupCoordinator>(num_device_pairs);
  bool is_full = start_request.type() == StartBackupRequest::FULL_BACKUP;
  std::string backup_uuid = make_uuid();

  std::shared_ptr<BackupEventHandler> event_handler =
      status_tracker_->CreateEventHandler(backup_uuid);

  LOG(INFO) << "Adding to in progress set";
  this->AddToInProgressSet(backup_uuid, start_request, coordinator);
  LOG(INFO) << "Finished adding";
  std::shared_ptr<Backup> backup;
  try {
    LOG(INFO) << "Creating backup object";
     backup = backup_builder_->CreateBackup(device_pairs, coordinator,
                                            is_full);
    LOG(INFO) << "Created backup object";
  } catch (...) {
    std::lock_guard<std::mutex> map_lock(in_progress_map_mutex_);
    this->in_progress_map_.erase(backup_uuid);
    throw;
  }

  // mutable in order to release the backup shared_ptr
  std::thread backup_thread([=]() mutable {
    LOG(INFO) << "Starting backup " << backup_uuid;
    backup->DoBackup(event_handler);
    LOG(INFO) << "Finished backup " << backup_uuid;
    // Release the pointer when done to avoid a race on program exit
    // as we detach below
    backup.reset();

    std::lock_guard<std::mutex> map_lock(in_progress_map_mutex_);
    this->in_progress_map_.erase(backup_uuid);
  });

  backup_thread.detach();

  LOG(INFO) << "Returning from StartBackup " << backup_uuid;
  return backup_uuid;
}

void BackupManager::StopBackup(const StopBackupRequest &stop_request) {
  std::lock_guard<std::mutex> map_lock(in_progress_map_mutex_);

  std::string job_uuid = stop_request.job_uuid();

  if (in_progress_map_.count(job_uuid) == 0) {
    LOG(ERROR) << "Job with UUID '" << stop_request.job_uuid()
               << "' is not running";
    throw BackupException("Job not running");
  }

  auto coordinator = in_progress_map_[job_uuid].second;
  coordinator->Cancel();
}

void BackupManager::AddToInProgressSet(
    std::string backup_uuid,
    const StartBackupRequest &start_backup_request,
    std::shared_ptr<BackupCoordinator> coordinator) {

  std::lock_guard<std::mutex> map_lock(in_progress_map_mutex_);

  std::set<std::string> to_backup_uuids;
  for (DevicePair device_pair : start_backup_request.device_pairs()) {
    to_backup_uuids.insert(device_pair.block_device_uuid());
  }

  // Make sure we aren't backing up any UUID already in progress
  for (auto in_progress_pair : in_progress_map_) {
    // See backup_manager.h comment for in_progress_map_
    std::set<std::string> uuids = in_progress_pair.second.first;
    for (std::string in_progress_uuid : uuids) {
      if (to_backup_uuids.count(in_progress_uuid) > 0) {
        LOG(ERROR) << "UUID '" << in_progress_uuid << "'"
                   << " is already in progress";
        throw BackupException("Device is already being backed up");
      }
    }
  }

  std::pair<std::set<std::string>, std::shared_ptr<BackupCoordinator>>
  in_progress_pair(to_backup_uuids, coordinator);

  in_progress_map_[backup_uuid] = in_progress_pair;
}

BackupManager::~BackupManager() {
  destructor_called_ = true;
  while (in_progress_map_.size() != 0U) {
    std::this_thread::yield();
  }
}

} // datto_linux_client
