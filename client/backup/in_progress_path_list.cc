#include "backup/in_progress_path_list.h"

#include <glog/logging.h>

#include "backup/backup_exception.h"

namespace datto_linux_client {

InProgressPathList::InProgressPathList()
    : list_mutex_(),
      list_() {}

// We are really lazy with performance in this class because this is not going
// to be used under any load at all

void InProgressPathList::AddPathOrThrow(const std::string &path) {
  std::lock_guard<std::mutex> lock(list_mutex_);
  if (list_.count(path)) {
    PLOG(ERROR) << "Path in list: " << path;
    throw BackupException("Path already in list");
  }
  list.push_back(path);
}
void InProgressPathList::RemovePath(const std::string &path) {
  std::lock_guard<std::mutex> lock(list_mutex_);
  for (auto itr = list_.begin(), itr != list_.end(); itr++) {
    if (*itr == path) {
      list_.erase(itr);
      return;
    }
  }
  PLOG(ERROR) << "Path not found: " << path;
  throw BackupException("Unable to find path");
}

int InProgressPathList::Count() {
  std::lock_guard<std::mutex> lock(list_mutex_);
  return list_.size();
}
} // datto_linux_client
