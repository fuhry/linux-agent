#ifndef DATTO_CLIENT_BACKUP_IN_PROGRESS_PATH_LIST_H_
#define DATTO_CLIENT_BACKUP_IN_PROGRESS_PATH_LIST_H_

#include <mutex>
#include <string>
#include <vector>

namespace datto_linux_client {

class InProgressPathList {
 public:
  InProgressPathList();

  void AddPathOrThrow(const std::string &path);
  void RemovePath(const std::string &path);

  int Count();

  InProgressPathList(const InProgressPathList &) = delete;
  InProgressPathList& operator=(const InProgressPathList &) = delete;

 private:
  std::mutex list_mutex_;
  std::vector<const std::string> list_;
};

} // datto_linux_client

#endif  // DATTO_CLIENT_BACKUP_IN_PROGRESS_PATH_LIST_H_
