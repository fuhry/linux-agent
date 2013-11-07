#include "backup/backup_event_handler.h"

// TODO: This file
namespace datto_linux_client {

BackupEventHandler::BackupEventHandler(const std::string &job_guid) {}

void BackupEventHandler::BackupPreparing() {

}

void BackupEventHandler::BackupCopying() {

}

void BackupEventHandler::BackupSucceeded() {
}

void BackupEventHandler::BackupFailed(const std::string &failure_message) {
}

void BackupEventHandler::UpdateUnsyncedCount(uint64_t num_unsynced) {

}

} // datto_linux_client
