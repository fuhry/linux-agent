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

void BackupEventHandler::BackupInProgress() {
  std::lock_guard<std::mutex> lock(*to_lock_mutex_);
  reply_->set_status(BackupStatusReply::IN_PROGRESS);
}

void BackupEventHandler::BackupCancelled() {
  std::lock_guard<std::mutex> lock(*to_lock_mutex_);
  reply_->set_status(BackupStatusReply::CANCELLED);
}

void BackupEventHandler::BackupSucceeded() {
  std::lock_guard<std::mutex> lock(*to_lock_mutex_);
  reply_->set_status(BackupStatusReply::SUCCEEDED);
}

void BackupEventHandler::BackupFailed(const std::string &failure_message) {
  std::lock_guard<std::mutex> lock(*to_lock_mutex_);
  reply_->set_status(BackupStatusReply::FAILED);
  reply_->set_failure_message(failure_message);
}

std::shared_ptr<SyncCountHandler> BackupEventHandler::CreateSyncCountHandler(
    const std::string &block_device_name) {
  return nullptr;
}

} // datto_linux_client
