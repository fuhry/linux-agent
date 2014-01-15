#include "dattod/flock.h"

#include <fstream>

#include <sys/types.h>
#include <unistd.h>

#include <gtest/gtest.h>
#include <glog/logging.h>

#include <stdexcept>

namespace {

using ::datto_linux_client::Flock;

TEST(FlockTest, Constructor) {
  Flock lock("/tmp/test_lock");
}

TEST(FlockTest, FailsOnBadPath) {
  try {
    Flock lock("/dir_doesnt_exist/test_lock");
    FAIL() << "Locking on bad file didn't fail";
  } catch (const std::runtime_error &e) {
    // good
  }
}

TEST(FlockTest, FailsOnAlreadyLocked) {
  Flock lock1("/tmp/test_lock");
  try {
    Flock lock2("/tmp/test_lock");
    FAIL() << "Locking on locked file didn't fail";
  } catch (const std::runtime_error &e) {
    // good
  }
}

TEST(FlockTest, ReleasesInDestructor) {
  {
    Flock lock1("/tmp/test_lock");
  }
  Flock lock2("/tmp/test_lock");
}

TEST(FlockTest, WritePID) {
  Flock lock("/tmp/test_lock");
  lock.WritePid();
  std::ifstream lock_ifs("/tmp/test_lock");
  pid_t file_pid;

  lock_ifs >> file_pid;

  EXPECT_EQ(file_pid, getpid());
}

} // namespace
