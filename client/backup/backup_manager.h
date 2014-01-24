#ifndef DATTO_CLIENT_BACKUP_BACKUP_MANAGER_H_
#define DATTO_CLIENT_BACKUP_BACKUP_MANAGER_H_

#include <atomic>
#include <memory>

#include "backup/backup.h"
#include "backup/backup_builder.h"
#include "backup/in_progress_path_set.h"
#include "backup_status_tracker/backup_status_tracker.h"
#include "unsynced_sector_manager/unsynced_sector_manager.h"

#include "device_pair.pb.h"
#include "request.pb.h"
#include "reply.pb.h"
#include "backup_status_request.pb.h"
#include "start_backup_request.pb.h"
#include "stop_backup_request.pb.h"

namespace datto_linux_client {

class BackupManager {
 public:
  BackupManager(std::shared_ptr<BackupBuilder> backup_builder,
                std::shared_ptr<UnsyncedSectorManager> sector_manager,
                std::shared_ptr<BackupStatusTracker> status_tracker)
      : backup_builder_(backup_builder),
        sector_manager_(sector_manager),
        status_tracker_(status_tracker),
        destructor_called_(false) {}

  Reply StartBackup(const StartBackupRequest &start_request);
  Reply StopBackup(const StopBackupRequest &stop_request);
  Reply BackupStatus(const BackupStatusRequest &status_request);

  ~BackupManager();

  BackupManager (const BackupManager&) = delete;
  BackupManager& operator=(const BackupManager&) = delete;

 private:
  std::shared_ptr<BackupBuilder> backup_builder_;
  std::shared_ptr<UnsyncedSectorManager> sector_manager_;
  std::shared_ptr<BackupStatusTracker> status_tracker_;

  std::atomic<bool> destructor_called_;
};

} // datto_linux_client

#endif  //  DATTO_CLIENT_BACKUP_BACKUP_MANAGER_H_
