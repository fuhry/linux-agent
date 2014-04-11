#include "backup/backup.h"

#include <thread>

#include <glog/logging.h>

#include "backup/backup_exception.h"
#include "backup_status_tracker/sync_count_handler.h"

namespace datto_linux_client {

Backup::Backup(
    std::vector<std::shared_ptr<DeviceSynchronizerInterface>> syncs_to_do,
    std::shared_ptr<BackupCoordinator> coordinator)
    : syncs_to_do_(syncs_to_do),
      coordinator_(coordinator) {}

void Backup::DoBackup(std::shared_ptr<BackupEventHandler> event_handler) {
  event_handler->BackupInProgress();
  std::vector<std::thread> in_progress_threads;
  for (auto sync : syncs_to_do_) {
    std::shared_ptr<SyncCountHandler> sync_count_handler =
        event_handler->CreateSyncCountHandler(*sync->source_device());

    std::thread sync_thread([=]() {
      try {
        sync->DoSync(coordinator_, sync_count_handler);
      } catch (const std::exception &e) {
        std::string source_uuid;
        try {
          source_uuid = sync->source_device()->GetUuid();
        } catch (const std::exception &uuid_e) {
          LOG(ERROR) << uuid_e.what();
          source_uuid = "uuid_error";
        }
        BackupError error(std::string(e.what()), source_uuid);
        coordinator_->AddFatalError(error);
      }
      LOG(INFO) << "DeviceSynchronizer::DoSync returned";
    });

    in_progress_threads.push_back(std::move(sync_thread));
  }

  while (!coordinator_->WaitUntilFinished(2000)) {
    LOG(INFO) << "Waiting for backup to finish";
  }

  for (auto itr = in_progress_threads.begin();
       itr != in_progress_threads.end();
       ++itr) {
    itr->join();
  }

  std::string error_text;

  for (auto error : coordinator_->GetFatalErrors()) {
    if (error_text.size()) {
      error_text += "\n";
    }

    std::string source = error.error_source();
    if (source.size()) {
      error_text += source + " : ";
    }
    error_text += error.error_text();
  }

  if (error_text.size()) {
    event_handler->BackupFailed(error_text);
  } else if (coordinator_->IsCancelled()) {
    event_handler->BackupCancelled();
  } else {
    event_handler->BackupSucceeded();
  }
}

Backup::~Backup() { }

} // datto_linux_client
