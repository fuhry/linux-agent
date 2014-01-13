#include "backup_status_tracker/backup_event_handler.h"

namespace datto_linux_client {

BackupEventHandler::BackupEventHandler(
    const std::string &job_uuid,
    std::shared_ptr<std::mutex> to_lock_mutex,
    std::shared_ptr<BackupStatusReply> reply)
  : job_uuid_(job_uuid),
    to_lock_mutex_(to_lock_mutex),
    reply_(reply) {
  std::lock_guard<std::mutex> lock(*to_lock_mutex_);
  reply_->set_status(BackupStatusReply::NOT_STARTED);
}

void BackupEventHandler::BackupCopying() {
  std::lock_guard<std::mutex> lock(*to_lock_mutex_);
  reply_->set_status(BackupStatusReply::COPYING);
}

void BackupEventHandler::BackupCancelled() {
  std::lock_guard<std::mutex> lock(*to_lock_mutex_);
  reply_->set_status(BackupStatusReply::CANCELLED);
}

void BackupEventHandler::BackupFinished() {
  std::lock_guard<std::mutex> lock(*to_lock_mutex_);
  reply_->set_status(BackupStatusReply::FINISHED);
}

void BackupEventHandler::BackupFailed(const std::string &failure_message) {
  std::lock_guard<std::mutex> lock(*to_lock_mutex_);
  reply_->set_status(BackupStatusReply::FAILED);
  reply_->set_failure_message(failure_message);
}

void BackupEventHandler::UpdateSyncedCount(uint64_t num_synced) {
  std::lock_guard<std::mutex> lock(*to_lock_mutex_);
  reply_->set_bytes_transferred(num_synced);
}

void BackupEventHandler::UpdateUnsyncedCount(uint64_t num_unsynced) {
  // TODO: no-op right now
}

} // datto_linux_client
