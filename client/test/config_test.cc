#include "configuration/config.h"

#include <functional>

// chroot, fork, wait, mkdir, open, mmap
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string>

#include <gtest/gtest.h>
#include <glog/logging.h>

namespace {

  using namespace std;

using ::datto_linux_client::Config;
using ::datto_linux_client::DEFAULT_CONFIG_PATH;
using ::datto_linux_client::DEFAULT_CONFIG_DIR;

const string real_target_dir("/tmp/datto_config_test/");
const string source_dir("./test/data/configs/");

void MakeDirectoryIfNotExist(const char *dir_path) {
  mode_t dir_mode = S_IRWXU;
  if (mkdir(dir_path, dir_mode)) {
    if (errno != EEXIST) {
      PLOG(ERROR) << "Unable to make /tmp/datto_config_test";
      throw std::runtime_error("Error making directory");
    }
  }
}

void CopyConfig (const string & config_file) {

  MakeDirectoryIfNotExist(real_target_dir.c_str());

  string source = source_dir + config_file;
  string dest   = real_target_dir + config_file;
  string cp_cmd = "/usr/bin/cp " + source + " " + dest;

  if ( system(cp_cmd.c_str()) ) {
    string err = string("Error copying ")  + config_file + " into target directory";
    PLOG(ERROR) << err;
    throw std::runtime_error(err);
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

//
// Test finding the default config (/etc/datto/dattod.conf)
//

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

//
// Test finding/loading a config file by name
//

TEST(ConfigTest, LoadsNamedConfig) {
  std::function<void()> find_named = [&]() {
    MakeDirectoryIfNotExist(DEFAULT_CONFIG_DIR);

    const string named_config_file = "named.conf";

    // Open and close a named file to make an empty file

    string named_config_path = string(DEFAULT_CONFIG_DIR) + "/" + named_config_file;
    mode_t conf_mode = S_IRWXU;
    int conf_fd = open(named_config_path.c_str(), O_CREAT | O_RDWR, conf_mode);
    if (conf_fd == -1) {
      PLOG(ERROR) << "Unable to create named config " << named_config_path;
      FAIL();
    }
    close(conf_fd);

    Config::LoadConfigFile(named_config_path);
  };

  // find_named shouldn't call any gtest functions, just exit(1)
  RunInChroot(find_named);
}

//
// Test to make sure that trying to load a second config file fails
//

TEST(ConfigTest, NoReloadsAllowed) {
  std::function<void()> check_reload = [&]() {
    MakeDirectoryIfNotExist(DEFAULT_CONFIG_DIR);

    const string named_config_file = "named.conf";

    // Open and close a named file to make an empty file

    string named_config_path = string(DEFAULT_CONFIG_DIR) + "/" + named_config_file;
    mode_t conf_mode = S_IRWXU;
    int conf_fd = open(named_config_path.c_str(), O_CREAT | O_RDWR, conf_mode);
    if (conf_fd == -1) {
      PLOG(ERROR) << "Unable to open named config " << named_config_path;
      FAIL();
    }
    close(conf_fd);

    try {
      Config::LoadConfigFile(named_config_path);
    }
    catch (...) {
      PLOG(ERROR) << "Initial load of config file failed";
      FAIL();
    }

    try {
      Config::LoadConfigFile(named_config_path);
      PLOG(ERROR) << "Attempted reload of config file did NOT throw an exception";
      FAIL();
    }
    catch (...) {
    }


  };

  // find_named shouldn't call any gtest functions, just exit(1)
  RunInChroot(check_reload);
}

//
// Test loading and accessing a string value
//

TEST(ConfigTest, StringLoad) {

  std::function<void()> string_load = [&]() {

    string named_config_path = string(DEFAULT_CONFIG_DIR) + "/goodstring.conf";
    Config::LoadConfigFile(named_config_path);

    EXPECT_EQ("a_value", Config::GetString("a_key"));

  };
  
  CopyConfig("goodstring.conf");

  RunInChroot(string_load);

}

//
// Test loading a config file that contains a bad line (verify exception throw)
//

TEST(ConfigTest, BadLine) {

  std::function<void()> invalid_lines = [&]() {

    string named_config_path = string(DEFAULT_CONFIG_DIR) + "/invalidlines.conf";
    try {
      Config::LoadConfigFile(named_config_path);
      PLOG(ERROR) << "Error: config file with bad line did not throw exception";
      FAIL();
    }
    catch (...) {
    }

  };
  
  CopyConfig("invalidlines.conf");

  RunInChroot(invalid_lines);

}


//
// Test loading and accessing uint64_t values 
//

TEST(ConfigTest, Unsigned_int64) {

  std::function<void()> uint64_load = [&]() {

    string named_config_path = string(DEFAULT_CONFIG_DIR) + "/numbers.conf";
    Config::LoadConfigFile(named_config_path);
    
    // Test good one first
    EXPECT_EQ(uint64_t(123456789), Config::GetUInt64("gooduint64"));

    uint64_t should_not_work;
    should_not_work = should_not_work;

    try {
      should_not_work = Config::GetUInt64("baduint64a");
      PLOG(ERROR) << "Error, did not catch trailing junk on GetUInt64()";
      FAIL();
    }
    catch (...) {
    }

    try {
      should_not_work = Config::GetUInt64("baduint64b");
      PLOG(ERROR) << "Error, did not catch non-numeric value on GetUInt64()";
      FAIL();
    }
    catch (...) {
    }

  };
  
  CopyConfig("numbers.conf");

  RunInChroot(uint64_load);

}

//
// Test loading and accessing int64_t values 
//

TEST(ConfigTest, Signed_int64) {

  std::function<void()> int64_load = [&]() {

    string named_config_path = string(DEFAULT_CONFIG_DIR) + "/numbers.conf";
    Config::LoadConfigFile(named_config_path);
    
    // Test good one first
    EXPECT_EQ(int64_t(-123456789),  Config::GetInt64("goodint64"));

    int64_t should_not_work;
    should_not_work = should_not_work;

    try {
      should_not_work = Config::GetInt64("badint64a");
      PLOG(ERROR) << "Error, did not catch trailing junk on GetInt64()";
      FAIL();
    }
    catch (...) {
    }

    try {
      should_not_work = Config::GetInt64("badint64b");
      PLOG(ERROR) << "Error, did not catch non-numeric value on GetInt64()";
      FAIL();
    }
    catch (...) {
    }

  };
  
  CopyConfig("numbers.conf");

  RunInChroot(int64_load);

}

//
// Test loading and accessing int32_t values 
//

TEST(ConfigTest, Signed_int32) {

  std::function<void()> int32_load = [&]() {

    string named_config_path = string(DEFAULT_CONFIG_DIR) + "/numbers.conf";
    Config::LoadConfigFile(named_config_path);
    
    // Test good one first
    EXPECT_EQ(int32_t(-123456),  Config::GetInt32("goodint32"));

    int32_t should_not_work;
    should_not_work = should_not_work;

    try {
      should_not_work = Config::GetInt32("badint32a");
      PLOG(ERROR) << "Error, did not catch trailing junk on GetInt32()";
      FAIL();
    }
    catch (...) {
    }

    try {
      should_not_work = Config::GetInt32("badint32b");
      PLOG(ERROR) << "Error, did not catch non-numeric value on GetInt32()";
      FAIL();
    }
    catch (...) {
    }

  };
  
  CopyConfig("numbers.conf");

  RunInChroot(int32_load);

}

//
// Test loading and accessing int32_t values 
//

TEST(ConfigTest, Unsigned_int32) {

  std::function<void()> uint32_load = [&]() {

    string named_config_path = string(DEFAULT_CONFIG_DIR) + "/numbers.conf";
    Config::LoadConfigFile(named_config_path);
    
    // Test good one first
    EXPECT_EQ(uint32_t(123456),  Config::GetUInt32("gooduint32"));

    uint32_t should_not_work;
    should_not_work = should_not_work;

    try {
      should_not_work = Config::GetUInt32("baduint32a");
      PLOG(ERROR) << "Error, did not catch trailing junk on GetUInt32()";
      FAIL();
    }
    catch (...) {
    }

    try {
      should_not_work = Config::GetUInt32("baduint32b");
      PLOG(ERROR) << "Error, did not catch non-numeric value on GetUInt32()";
      FAIL();
    }
    catch (...) {
    }

  };
  
  CopyConfig("numbers.conf");

  RunInChroot(uint32_load);

}

//
// Test loading and accessing double values 
//

TEST(ConfigTest, Double) {

  std::function<void()> double_load = [&]() {

    string named_config_path = string(DEFAULT_CONFIG_DIR) + "/numbers.conf";
    Config::LoadConfigFile(named_config_path);
    
    // Test good one first
    EXPECT_EQ(double(3.1416),  Config::GetDouble("gooddouble"));

    double should_not_work;
    should_not_work = should_not_work;

    try {
      should_not_work = Config::GetDouble("baddoublea");
      PLOG(ERROR) << "Error, did not catch trailing junk on GetDouble()";
      FAIL();
    }
    catch (...) {
    }

    try {
      should_not_work = Config::GetDouble("baddoubleb");
      PLOG(ERROR) << "Error, did not catch non-numeric value on GetDouble()";
      FAIL();
    }
    catch (...) {
    }

  };
  
  CopyConfig("numbers.conf");

  RunInChroot(double_load);

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
