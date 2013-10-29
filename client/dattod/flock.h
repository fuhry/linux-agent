#ifndef DATTO_CLIENT_DATTOD_FLOCK_H_
#define DATTO_CLIENT_DATTOD_FLOCK_H_

#include <string>

namespace datto_linux_client {

class Flock {
 public:
  Flock(std::string file_path);
  ~Flock();

  std::string path() const {
    return path_;
  }

  // Writes the current process PID to the lock file
  void WritePid();

  Flock(const Flock&) = delete;
  Flock& operator=(const Flock&) = delete;
 private:
  std::string path_;
  int path_fd_;
};
} // datto_linux_client

#endif //  DATTO_CLIENT_DATTOD_FLOCK_H_
