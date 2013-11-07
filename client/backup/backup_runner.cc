#include "backup/backup_runner.h"
#include <glog/logging.h>

namespace datto_linux_client {

BackupRunner::BackupRunner(
    std::shared_ptr<Backup> backup,
    std::shared_ptr<BackupRunnerTracker> backup_runner_tracker,
    std::shared_ptr<CancellationToken> cancel_token)
    : backup_(backup),
      backup_runner_tracker_(backup_runner_tracker),
      cancel_token_(cancel_token) {

  backup_thread_ = std::thread([&]() {
    try {
      backup_->DoBackup(cancel_token_);
    } catch (const std::runtime_error &e) {
      LOG(ERROR) << e.what();
    }
    backup_runner_tracker->RunnerComplete(this);
  });
}

BackupRunner::~BackupRunner() {
  backup_thread_.join();
}

} // datto_linux_client

#endif  // DATTO_CLIENT_BACKUP_BACKUP_RUNNER_H_
