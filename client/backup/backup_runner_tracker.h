#ifndef DATTO_LINUX_CLIENT_BACKUP_BACKUP_RUNNER_TRACKER_H_
#define DATTO_LINUX_CLIENT_BACKUP_BACKUP_RUNNER_TRACKER_H_

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "backup/backup_runner.h"

namespace datto_linux_client {

// This class exists as a storage location for in-progress BackupRunners. When
// a BackupRunner is complete (sucess, cancellation, failure or otherwise), it
// should call RunnerComplete with "this" as the argument
//
// The reason we bother with this instead of using detached threads is to
// allow for controlled clean up when tearing everything (or anything) down
class BackupRunnerTracker {
 public:
  BackupRunnerTracker();

  void StartRunner(std::unique_ptr<Backup> backup,
                   std::shared_ptr<CancellationToken> cancel_token);

  // ClearBackup never throws
  void RunnerComplete(const BackupRunner &runner);
 private:
  std::mutex in_progress_mutex_;
  std::vector<const std::unique_ptr<BackupRunner>> in_progress_list_;
};
} // datto_linux_client

#endif //  DATTO_LINUX_CLIENT_BACKUP_BACKUP_RUNNER_TRACKER_H_
