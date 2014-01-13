#ifndef DATTO_CLIENT_STATUS_TRACKER_STATUS_TRACKER_H_
#define DATTO_CLIENT_STATUS_TRACKER_STATUS_TRACKER_H_

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "backup_status_tracker/backup_event_handler.h"
#include "backup_status_reply.pb.h"

namespace datto_linux_client {

class BackupStatusTracker {
 public:
  BackupStatusTracker();
  ~BackupStatusTracker() {}

  // Returns a nullptr when the job_uuid doesn't have an associated reply
  std::unique_ptr<BackupStatusReply> GetReply(const std::string &job_uuid);

  // If an event handler for a job uuid exists an exception is thrown
  std::unique_ptr<BackupEventHandler> CreateEventHandler(
      const std::string &job_uuid);

 private:
  std::shared_ptr<std::mutex> map_mutex_;
  std::map<std::string, std::shared_ptr<BackupStatusReply>> reply_map_;

};

}

#endif //  DATTO_CLIENT_STATUS_TRACKER_STATUS_TRACKER_H_
