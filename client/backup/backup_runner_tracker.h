#ifndef DATTO_LINUX_CLIENT_BACKUP_BACKUP_RUNNER_TRACKER_H_
#define DATTO_LINUX_CLIENT_BACKUP_BACKUP_RUNNER_TRACKER_H_

#include <memory>
#include <string>
#include <vector>

namespace datto_linux_client {

// This class exists as a storage location for in-progress BackupRunners. When
// a BackupRunner is complete (sucess, cancellation, failure or otherwise), it
// should call RunnerComplete with "this" as the argument
class BackupRunnerTracker {
 public:
  BackupRunnerTracker();

  void AddRunner(std::unique_ptr<BackupRunner> runner);
  // ClearBackup never throws
  void RunnerComplete(const BackupRunner &runner);
 private:
  std::mutex in_progress_mutex_;
  std::vector<const std::unique_ptr<Backup>> in_progress_list_;
};
} // datto_linux_client

#endif //  DATTO_LINUX_CLIENT_BACKUP_BACKUP_RUNNER_TRACKER_H_
