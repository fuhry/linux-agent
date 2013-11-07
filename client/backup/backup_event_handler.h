#ifndef DATTO_CLIENT_BACKUP_BACKUP_EVENT_HANDLER_H_
#define DATTO_CLIENT_BACKUP_BACKUP_EVENT_HANDLER_H_

#include <string>
#include <functional>
#include <unistd.h>

namespace datto_linux_client {

enum BackupStatus {
  NOT_STARTED = 0,
  PREPARING,
  COPYING,
  CLEANING_UP,
  FINISHED,
  FAILED,
};

class BackupEventHandler {
 public:
  BackupEventHandler(const std::string &job_guid);

  void BackupPreparing();
  void BackupCopying();

  void BackupSucceeded();
  void BackupCancelled();
  void BackupFailed(const std::string &failure_message);

  void UpdateUnsyncedCount(uint64_t num_unsynced);

  BackupEventHandler(const BackupEventHandler &) = delete;
  BackupEventHandler& operator=(const BackupEventHandler &) = delete;
 private:
  const std::string job_guid_;
};

} // datto_linux_client

#endif  // DATTO_CLIENT_BACKUP_BACKUP_EVENT_HANDLER_H_
