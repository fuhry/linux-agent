#include "backup/backup.h"
#include "backup/backup_exception.h"

#include <thread>

#include <glog/logging.h>

namespace datto_linux_client {

Backup::Backup(
    std::vector<std::shared_ptr<DeviceSynchronizerInterface>> syncs_to_do) {}

void Backup::DoBackup(std::shared_ptr<BackupEventHandler> event_handler) {
  if (!coordinator_) {
    // TODO
    coordinator_ = nullptr;
  }
}

void Backup::InsertBackupCoordinator(
    std::shared_ptr<BackupCoordinator> coordinator) {
  coordinator_ = coordinator;
}


Backup::~Backup() { }

} // datto_linux_client
