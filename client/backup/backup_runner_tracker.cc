#include "backup/backups_in_progress_map.h"

#include <thread>

namespace datto_linux_client {

BackupRunnerTracker::BackupRunnerTracker()
    : in_progress_mutex_(),
      in_progress_list_() {}

void BackupRunnerTracker::AddRunner(std::unique_ptr<BackupRunner> runner) { 
  std::lock_guard<std::mutex> lock(in_progress_mutex_);
  in_progress_list_.push_back(std::move(runner));
}
void BackupRunnerTracker::RunnerComplete(const *BackupRunner runner) {

  // We worry about this because calling erase() calls the destructor for the
  // BackupRunner. A thread in the BackupRunner is the one calling
  // RunnerComplete. The destructor for BackupRunner blocks on the thread
  // calling this, which will cause us to deadlock if we don't call erase in a
  // separate thread.
  std::thread delete_thread([&]() {
    std::lock_guard<std::mutex> lock(in_progress_mutex_);
    for (auto itr = in_progress_list_.begin();
         itr != in_progress_list_.end();
         itr++) {
      if (itr->get() == runner) {
        in_progress_list_.erase(itr);
        break;
      }
    }
  };

  delete_thread.detach();
}

} // datto_linux_client

