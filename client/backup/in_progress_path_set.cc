#include "backup/in_progress_path_set.h"

#include <glog/logging.h>

#include "backup/backup_exception.h"

namespace datto_linux_client {

InProgressPathSet::InProgressPathSet()
    : set_mutex_(),
      set_() {}

// We are really lazy with performance in this class because this is not going
// to be used under any load at all

void InProgressPathSet::AddPathOrThrow(const std::string &path) {
  std::lock_guard<std::mutex> lock(set_mutex_);
  if (set_.count(path)) {
    PLOG(ERROR) << "Path in set: " << path;
    throw BackupException("Path already in set");
  }
  set_.insert(path);
}
void InProgressPathSet::RemovePath(const std::string &path) {
  std::lock_guard<std::mutex> lock(set_mutex_);
  if (set_.erase(path) == 0) {
    PLOG(ERROR) << "Path not found: " << path;
    throw BackupException("Unable to find path");
  }
}

int InProgressPathSet::Count() {
  std::lock_guard<std::mutex> lock(set_mutex_);
  return set_.size();
}
} // datto_linux_client
