#include "backup_status_tracker/backup_status_tracker.h"

namespace datto_linux_client {

BackupStatusTracker::BackupStatusTracker() : reply_map_() {
  map_mutex_ = std::make_shared<std::mutex>();
}

std::shared_ptr<BackupStatusReply> BackupStatusTracker::GetReply(
    const std::string &job_uuid) {
  std::lock_guard<std::mutex> lock(*map_mutex_);

  if (!reply_map_.count(job_uuid)) {
    return nullptr;
  }

  auto reply = std::make_shared<BackupStatusReply>(*reply_map_[job_uuid]);
  return reply;
}

std::shared_ptr<BackupEventHandler> BackupStatusTracker::CreateEventHandler(
    const std::string &job_uuid) {

  std::shared_ptr<BackupStatusReply> reply;
  {
    std::lock_guard<std::mutex> lock(*map_mutex_);

    if (reply_map_.count(job_uuid)) {
      // TODO Exception
      return nullptr;
    }

    reply = std::make_shared<BackupStatusReply>();
    reply_map_[job_uuid] = reply;
  }
  
  std::shared_ptr<BackupEventHandler> handler(
      new BackupEventHandler(map_mutex_, reply));

  return std::move(handler);
}

}
