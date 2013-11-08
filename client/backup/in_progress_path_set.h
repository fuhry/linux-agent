#ifndef DATTO_CLIENT_BACKUP_IN_PROGRESS_PATH_SET_H_
#define DATTO_CLIENT_BACKUP_IN_PROGRESS_PATH_SET_H_

#include <mutex>
#include <string>
#include <set>

namespace datto_linux_client {

class InProgressPathSet {
 public:
  InProgressPathSet();

  void AddPathOrThrow(const std::string &path);
  void RemovePath(const std::string &path);

  int Count();

  InProgressPathSet(const InProgressPathSet &) = delete;
  InProgressPathSet& operator=(const InProgressPathSet &) = delete;

 private:
  std::mutex set_mutex_;
  std::set<std::string> set_;
};

} // datto_linux_client

#endif  // DATTO_CLIENT_BACKUP_IN_PROGRESS_PATH_SET_H_
