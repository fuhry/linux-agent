#ifndef DATTO_CLIENT_BACKUP_BACKUP_EVENT_HANDLER_H_
#define DATTO_CLIENT_BACKUP_BACKUP_EVENT_HANDLER_H_

#include <memory>
#include <mutex>
#include <string>
#include <unistd.h>

#include "backup_status_reply.pb.h"

namespace datto_linux_client {

class BackupEventHandler {
 public:
  BackupEventHandler(const std::string &job_uuid,
                     std::shared_ptr<std::mutex> to_lock_mutex,
                     std::shared_ptr<BackupStatusReply> reply);
  virtual ~BackupEventHandler() {}

  virtual void BackupCopying();

  virtual void BackupFinished();
  virtual void BackupCancelled();
  virtual void BackupFailed(const std::string &failure_message);

  // num_synced should be the total synced
  virtual void UpdateSyncedCount(uint64_t num_synced);
  // num_unsynced should be the total synced
  virtual void UpdateUnsyncedCount(uint64_t num_unsynced);

  BackupEventHandler(const BackupEventHandler &) = delete;
  BackupEventHandler& operator=(const BackupEventHandler &) = delete;

 protected:
  // For unit testing
  BackupEventHandler() {}

 private:
  const std::string job_uuid_;
  std::shared_ptr<std::mutex> to_lock_mutex_;
  std::shared_ptr<BackupStatusReply> reply_;
};

} // datto_linux_client

#endif  // DATTO_CLIENT_BACKUP_BACKUP_EVENT_HANDLER_H_
