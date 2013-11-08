#ifndef DATTO_CLIENT_BACKUP_BACKUP_MANAGER_H_
#define DATTO_CLIENT_BACKUP_BACKUP_MANAGER_H_

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "backup/backup.h"
#include "backup/backup_runner.h"
#include "backup/backup_runner_tracker.h"
#include "block_device/block_device.h"
#include "unsynced_sector_manager/unsynced_sector_manager.h"

#include "request.pb.h"
#include "reply.pb.h"
#include "start_backup_request.pb.h"
#include "stop_backup_request.pb.h"

namespace datto_linux_client {

class BackupManager {
 public:
  BackupManager();

  Reply StartBackup(const StartBackupRequest &start_request);
  Reply StopBackup(const StopBackupRequest &stop_request);

  ~BackupManager();

  // Make sure there isn't a backup in progress
  if (in_progress_backups_.count(source_path)) {
    throw BackupException("Backup is already in progress");
  }


  BackupManager (const BackupManager&) = delete;
  BackupManager& operator=(const BackupManager&) = delete;

 private:
  BackupRunnerTracker backup_runner_tracker_;

  std::mutex cancel_tokens_mutex_;
  std::map<const std::string, std::weak_ptr<CancellationToken>>
      cancel_tokens_;

  std::mutex managers_mutex_;
  std::map<const std::string, std::shared_ptr<UnsyncedSectorManager>>
      unsynced_managers_;
};

} // datto_linux_client

#endif  //  DATTO_CLIENT_BACKUP_BACKUP_MANAGER_H_
