#include "backup_status_tracker/backup_status_tracker.h"

namespace datto_linux_client {

BackupStatusTracker::BackupStatusTracker() : reply_map_() {
  map_mutex_ = std::make_shared<std::mutex>();
}

std::shared_ptr<BackupStatusReply> BackupStatusTracker::GetReply(
    const std::string &job_uuid) {
  std::lock_guard<std::mutex> lock(*map_mutex_);

  std::shared_ptr<BackupStatusReply> reply(nullptr);

  if (reply_map_.count(job_uuid)) {
    // Create a copy of the reply
    reply.reset(new BackupStatusReply(*reply_map_[job_uuid]));
  }

  return std::move(reply);
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
      new BackupEventHandler(job_uuid, map_mutex_, reply));

  return std::move(handler);
}

}
