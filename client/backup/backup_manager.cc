#include "backup/backup_manager.h"

#include <stdint.h>
#include <uuid/uuid.h>
#include <string>

#include "backup/backup_exception.h"

#include <glog/logging.h>

namespace {
std::string make_uuid() {
  uuid_t uuid;
  uuid_generate_time(uuid);
  char uuid_c_str[36];
  uuid_unparse(uuid, uuid_c_str);
  return std::string(uuid_c_str);
}
} // anonymous namespace

namespace datto_linux_client {

Reply BackupManager::StartBackup(const StartBackupRequest &start_request) {
  Reply reply;
  return reply;
}

Reply BackupManager::StopBackup(const StopBackupRequest &stop_request) {
  Reply reply;
  return reply;
}

Reply BackupManager::BackupStatus(const BackupStatusRequest &status_request) {
  Reply reply;
  return reply;
}

BackupManager::~BackupManager() {}

} // datto_linux_client
