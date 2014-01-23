#include "backup/backup.h"
#include "backup/backup_exception.h"
#include "backup_status_tracker/sync_count_handler.h"

#include <thread>

#include <glog/logging.h>

namespace datto_linux_client {

Backup::Backup(
    std::vector<std::shared_ptr<DeviceSynchronizerInterface>> syncs_to_do)
    : syncs_to_do_(syncs_to_do),
      coordinator_(nullptr) {}

void Backup::DoBackup(std::shared_ptr<BackupEventHandler> event_handler) {
  if (!coordinator_) {
    coordinator_ = std::make_shared<BackupCoordinator>(syncs_to_do_.size());
  }

  event_handler->BackupInProgress();
  std::vector<std::thread> in_progess_threads;
  for (auto sync : syncs_to_do_) {
    std::shared_ptr<SyncCountHandler> sync_count_handler =
        event_handler->CreateSyncCountHandler(*sync->source_device());

    std::thread sync_thread([=]() {
      try {
        sync->DoSync(coordinator_, sync_count_handler);
      } catch (const std::exception &e) {
        BackupError error(std::string(e.what()),
                          sync->source_device()->uuid());
        coordinator_->AddFatalError(error);
      }
    });

    in_progess_threads.push_back(std::move(sync_thread));
  }

  while (!coordinator_->WaitUntilFinished(2000)) {
    LOG(INFO) << "Waiting for backup to finish";
  }

  for (auto itr = in_progess_threads.begin();
       itr != in_progess_threads.end();
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
  } else {
    event_handler->BackupSucceeded();
  }
}

void Backup::InsertBackupCoordinator(
    std::shared_ptr<BackupCoordinator> coordinator) {
  coordinator_ = coordinator;
}


Backup::~Backup() { }

} // datto_linux_client
