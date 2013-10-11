#include "configuration/config.h"

#include <functional>

// chroot, fork, wait, mkdir, open, mmap
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <gtest/gtest.h>
#include <glog/logging.h>

namespace {

using ::datto_linux_client::Config;
using ::datto_linux_client::DEFAULT_CONFIG_PATH;
using ::datto_linux_client::DEFAULT_CONFIG_DIR;

void MakeDirectoryIfNotExist(const char *dir_path) {
  mode_t dir_mode = S_IRWXU;
  if (mkdir(dir_path, dir_mode)) {
    if (errno != EEXIST) {
      PLOG(ERROR) << "Unable to make /tmp/datto_config_test";
      throw std::runtime_error("Error making directory");
    }
  }
}

void RunInChroot(std::function<void()> to_run) {
  // Make the temporary directory

  MakeDirectoryIfNotExist("/tmp/datto_config_test");
  MakeDirectoryIfNotExist("/tmp/datto_config_test/etc");

  int *is_done = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                             MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (is_done == MAP_FAILED) {
    PLOG(ERROR) << "mmap";
    FAIL();
  }

  // Fork so we can chroot with being locked in
  pid_t fork_ret = fork();

  // Error
  if (fork_ret < 0) {
    PLOG(ERROR) << "fork";
    FAIL();

  // Child
  } else if (fork_ret == 0) {
    if (chroot("/tmp/datto_config_test")) {
      PLOG(ERROR) << "chroot";
      _exit(1);
    }

    // Actually run the to_run function
    try {
      to_run();
      *is_done = 1;
    } catch (const std::runtime_error &e) {
      LOG(ERROR) << e.what();
      _exit(1);
    }

  // Parent
  } else {
    // Wait on the child to finish before returning
    int status = 1;
    if (wait(&status) == -1) {
      PLOG(ERROR) << "wait";
      FAIL();
    }
    if (!*is_done) {
      FAIL() << "chroot function didn't complete successfully";
    }

    munmap(is_done, sizeof(*is_done));
  }
}

TEST(ConfigTest, FindsDefaultConfig) {
  std::function<void()> find_default = [&]() {
    MakeDirectoryIfNotExist(DEFAULT_CONFIG_DIR);

    // Open and close to make an empty file
    mode_t conf_mode = S_IRWXU;
    int conf_fd = open(DEFAULT_CONFIG_PATH, O_CREAT | O_RDWR, conf_mode);
    if (conf_fd == -1) {
      PLOG(ERROR) << "Unable to open default config";
      FAIL();
    }
    close(conf_fd);

    Config::LoadDefaultConfig();
  };

  // find_default shouldn't call any gtest functions, just exit(1)
  RunInChroot(find_default);
}

TEST(ConfigTest, StringLoads) {
  Config::LoadConfigFile("./test/data/configs/strings.conf");

  EXPECT_EQ("a_value", Config::GetString("a_key"));
}

} // namespace

// For testing, make the directory client/test/data/configs
// Create the following unit tests and associated data files:
//
//
// U1. Finds default config
//  1. Define a lambda function that does the following:
//  1a. Create an empty file at the location of the default config path
//  1b. Make sure LoadDefaultConfig() succeeds 
//  2. Call above function with RunInChroot
//
// U2. Throw on no default config
//  1. Define a lambda function that does the following:
//  1a. Create an empty file at the location of the default config path
//  1b. Make sure LoadDefaultConfig() throws a ConfigException
//  2. Call above function with RunInChroot
//
// U3. Load non-default-config
//  1. Load the empty file client/test/data/configs/empty.conf
//  2. Make sure LoadConfigFile() succeeds
//
// U4. Throw on bad non-default-config
//  1. Make sure LoadConfigFile("/tmp/no_exist.conf") throws ConfigException
//
// U5. GetString() works
//  1. Load the non-default file client/test/data/configs/strings.conf
//  2. Make sure GetString("test_key") contains the expected value
//
// U6. Get*Int*() works
//  1. Load the non-default file client/test/data/configs/ints.conf
//  2. Make sure each Get*Int*("test_key") contains the expected value
//
// Other unit tests, time permitting, might include:
//   1. A file with spaces before the =, after the =, and before and after =
//   2. A file with nothing but blank lines
//   3. A couple files with bad data, that should throw ConfigExceptions
//   4. Loading the strings.conf file and trying to parse them as ints, making
//      sure it throws ConfigExceptions
