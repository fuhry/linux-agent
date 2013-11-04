#ifndef DATTO_CLIENT_BACKUP_BACKUP_MANAGER_H_
#define DATTO_CLIENT_BACKUP_BACKUP_MANAGER_H_

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "backup/backup.h"
#include "block_device/block_device.h"
#include "unsynced_sector_manager/unsynced_sector_manager.h"

#include "request.pb.h"
#include "start_backup_request.pb.h"
#include "stop_backup_request.pb.h"

namespace datto_linux_client {

class BackupManager {
 public:
  BackupManager();

  void StartFullBackup(const StartBackupRequest &start_request,
                       std::shared_ptr<ReplyChannel> reply_channel);

  void StartIncrementalBackup(const StartBackupRequest &start_request);
  
  void StopBackup(const StopBackupRequest &stop_request);

  ~BackupManager();

  BackupManager (const BackupManager&) = delete;
  BackupManager& operator=(const BackupManager&) = delete;

 private:
  std::mutex in_progress_mutex_;
  std::map<const std::string, std::unique_ptr<Backup>> in_progress_backups_;

  std::mutex managers_mutex_;
  std::map<const std::string, std::shared_ptr<UnsyncedSectorManager>>
      unsynced_managers_;
};

} // datto_linux_client

#endif  //  DATTO_CLIENT_BACKUP_BACKUP_MANAGER_H_
