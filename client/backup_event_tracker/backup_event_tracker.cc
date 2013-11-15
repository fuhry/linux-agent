#include "backup_event_tracker/backup_event_tracker.h"

namespace datto_linux_client {

BackupEventTracker::BackupEventTracker() : map_mutex_(), reply_map_() {
  map_mutex_ = std::make_shared<std::mutex>();
}

std::unique_ptr<BackupStatusReply> BackupEventTracker::GetReply(
    const std::string &job_guid) {
  std::lock_guard<std::mutex> lock(*map_mutex_);

  std::unique_ptr<BackupStatusReply> reply(nullptr);

  if (reply_map_.count(job_guid)) {
    // Create a copy of the reply
    reply.reset(new BackupStatusReply(*reply_map_[job_guid]));
  }

  return std::move(reply);
}

std::unique_ptr<BackupEventHandler> BackupEventTracker::CreateEventHandler(
    const std::string &job_guid) {

  std::shared_ptr<BackupStatusReply> reply;
  {
    std::lock_guard<std::mutex> lock(*map_mutex_);

    if (reply_map_.count(job_guid)) {
      // TODO Exception
      return nullptr;
    }

    reply = std::make_shared<BackupStatusReply>();
    reply_map_[job_guid] = reply;
  }
  
  std::unique_ptr<BackupEventHandler> handler(
      new BackupEventHandler(job_guid, map_mutex_, reply));

  return std::move(handler);
}


}