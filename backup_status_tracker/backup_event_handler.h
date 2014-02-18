#ifndef DATTO_CLIENT_BACKUP_STATUS_TRACKER_BACKUP_EVENT_HANDLER_H_
#define DATTO_CLIENT_BACKUP_STATUS_TRACKER_BACKUP_EVENT_HANDLER_H_

#include <memory>
#include <mutex>
#include <string>
#include <unistd.h>

#include "backup_status_reply.pb.h"
#include "backup_status_tracker/sync_count_handler.h"
#include "block_device/mountable_block_device.h"

namespace datto_linux_client {

class BackupEventHandler {
 public:
  BackupEventHandler(std::shared_ptr<std::mutex> to_lock_mutex,
                     std::shared_ptr<BackupStatusReply> reply);
  virtual ~BackupEventHandler() {}

  virtual void BackupInProgress();
  virtual void BackupSucceeded();
  virtual void BackupCancelled();
  virtual void BackupFailed(const std::string &failure_message);

  virtual std::shared_ptr<SyncCountHandler> CreateSyncCountHandler(
      const MountableBlockDevice &source_device);

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

#endif  // DATTO_CLIENT_BACKUP_STATUS_TRACKER_BACKUP_EVENT_HANDLER_H_
