#include "dattod/flock.h"
#include "dattod/dattod_exception.h"

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <glog/logging.h>

namespace datto_linux_client {

Flock::Flock(std::string file_path) : path_(file_path) {
  LOG(INFO) << "Attempting to flock " << file_path;

  path_fd_ = open(path_.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

  if (path_fd_ < 0) {
    PLOG(ERROR) << "Unable to open " << path_ << " for locking";
    throw DattodException("Unable to lock file");
  }

  if (flock(path_fd_, LOCK_EX | LOCK_NB)) {
    if (errno == EWOULDBLOCK) {
      LOG(ERROR) << "\"" << path_ << "\" is already locked!";
      throw DattodException("File already locked");
    }
    PLOG(ERROR) << "Failed to lock file";
    throw DattodException("Unknown lock failure");
  }
}

void Flock::WritePid() {
  DLOG(INFO) << "Writing PID to lock file";
  char buf[512];
  int pid_length = sprintf(buf, "%ld", (long)getpid());
  if (pid_length < 0) {
    throw DattodException("PID conversion failure");
  }
  if (ftruncate(path_fd_, 0)) {
    PLOG(ERROR) << "Unable to truncate lock file " << path_;
    throw DattodException("Unable to truncate lock file");
  }
  // + 1 for the \0
  if (write(path_fd_, buf, pid_length + 1) == -1) {
    PLOG(ERROR) << "Unable to write to lock file " << path_;
    throw DattodException("Unable to write to lock file");
  }
}

Flock::~Flock() {
  // Closing will also release the lock
  close(path_fd_);
}

}
