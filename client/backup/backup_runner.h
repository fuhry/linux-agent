#ifndef DATTO_CLIENT_BACKUP_BACKUP_RUNNER_H_
#define DATTO_CLIENT_BACKUP_BACKUP_RUNNER_H_

#include "backup/backup.h"
#include "backup/backups_in_progress_map.h"
#include "cancellation/cancellation_token.h"

namespace datto_linux_client {

// Constructing this will begin the Backup. When the Backup is finished,
// it is removed from the BackupRunnerTracker
class BackupRunner {
 public:
  BackupRunner(std::shared_ptr<Backup> backup,
               std::shared_ptr<BackupRunnerTracker> backup_runner_tracker,
               std::shared_ptr<CancellationToken> cancel_token);

  ~BackupRunner();

  BackupRunner(const BackupRunner &) = delete;
  BackupRunner& operator=(const BackupRunner &) = delete;
 private:
  std::shared_ptr<Backup> backup_;
  std::shared_ptr<BackupRunnerTracker> backup_runner_tracker_;
  std::shared_ptr<CancellationToken> cancel_token_;
};

} // datto_linux_client

#endif  // DATTO_CLIENT_BACKUP_BACKUP_RUNNER_H_
