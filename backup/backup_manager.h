#ifndef DATTO_CLIENT_BACKUP_BACKUP_MANAGER_H_
#define DATTO_CLIENT_BACKUP_BACKUP_MANAGER_H_

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <utility>

#include "backup/backup.h"
#include "backup/backup_builder.h"
#include "backup/backup_coordinator.h"
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

  // Returns the job UUID
  std::string StartBackup(const StartBackupRequest &start_request);
  void StopBackup(const StopBackupRequest &stop_request);

  ~BackupManager();

  BackupManager (const BackupManager&) = delete;
  BackupManager& operator=(const BackupManager&) = delete;

 private:
  void AddToInProgressSet(std::string backup_uuid,
                          const StartBackupRequest &start_backup_request,
                          std::shared_ptr<BackupCoordinator> coordinator);
  void CancelAll();

  std::mutex start_backup_mutex_;

  std::shared_ptr<BackupBuilder> backup_builder_;
  std::shared_ptr<UnsyncedSectorManager> sector_manager_;
  std::shared_ptr<BackupStatusTracker> status_tracker_;

  // backup_uuid -> (set of device uuids, backup_coordinator)
  std::map<std::string, std::pair<std::set<std::string>,
                                  std::shared_ptr<BackupCoordinator>>>
  in_progress_map_;
  std::mutex in_progress_map_mutex_;

  std::atomic<bool> destructor_called_;
};

} // datto_linux_client

#endif  //  DATTO_CLIENT_BACKUP_BACKUP_MANAGER_H_
