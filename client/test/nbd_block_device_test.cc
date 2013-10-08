#include "remote_block_device/nbd_block_device.h"
#include "test/loop_device.h"
#include <glog/logging.h>

#include <memory>
#include <gtest/gtest.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using ::datto_linux_client::NbdBlockDevice;

static const uint16_t LOCAL_TEST_PORT = 11235;
static const char LOCAL_TEST_HOST[] = "localhost";

namespace {

using ::datto_linux_client_test::LoopDevice;

class NbdBlockDeviceTest : public ::testing::Test {
 public:
 protected:
  class NbdServer {
   public:
    NbdServer(std::string file_to_serve) {
      pid_t fork_ret = fork();
      if (fork_ret == -1) {
          PLOG(ERROR) << "fork";
          throw std::runtime_error("fork failed");
      }
      else if (fork_ret == 0) {
          // This is the same as 2>&1 1>/dev/null
          int null_fd = open("/dev/null", O_RDWR);
          dup2(null_fd, 1);
          dup2(null_fd, 2);
          close(null_fd);
          // -d means it won't daemonize
          // -C /dev/null disables using a configuration file
          execl("/bin/nbd-server", "nbd-server",
                std::to_string(LOCAL_TEST_PORT).c_str(),
                file_to_serve.c_str(),
                "-d",
                "-C", "/dev/null",
                nullptr);
          // If we get here then exec failed
          PLOG(ERROR) << "execl";
          _exit(127);
      }
      else {
          nbd_server_pid = fork_ret;
          // Let the nbd_server start
          sleep(1);
      }
    }

    ~NbdServer() {
      if (kill(nbd_server_pid, SIGTERM)) {
        PLOG(ERROR) << "kill failed, check nbd-server was stopped correctly";
      } else {
        wait(nullptr);
      }
    }
   private:
    pid_t nbd_server_pid;
  };

  NbdBlockDeviceTest() {
    loop_dev = std::unique_ptr<LoopDevice>(new LoopDevice());
    // Let the loop device settle, might not be needed
    sleep(1);
    nbd_server = std::unique_ptr<NbdServer>(new NbdServer(loop_dev->path()));

    nbd_block_device = std::unique_ptr<NbdBlockDevice>(
        new NbdBlockDevice(LOCAL_TEST_HOST, LOCAL_TEST_PORT));
  }

  ~NbdBlockDeviceTest() { }

  std::unique_ptr<LoopDevice> loop_dev;
  std::unique_ptr<NbdServer> nbd_server;
  std::unique_ptr<NbdBlockDevice> nbd_block_device;
};

TEST_F(NbdBlockDeviceTest, CanConnect) {
  EXPECT_TRUE(nbd_block_device->IsConnected());
}

TEST(NbdBlockDeviceTestNoFixture, CantConnect) {
  try {
    auto nbd_block_device = std::unique_ptr<NbdBlockDevice>(
        new NbdBlockDevice(LOCAL_TEST_HOST, LOCAL_TEST_PORT));
    // Shouldn't get here as we didn't setup a server so the block device
    // should throw an exception
    FAIL();
  } catch (const std::runtime_error &e) {
    // Good
  } catch (...) {
    // Shouldn't be catching non-exceptions
    FAIL();
  }
}

TEST_F(NbdBlockDeviceTest, CanWrite) {
  int nbd_fd = nbd_block_device->Open();

  ssize_t bytes = write(nbd_fd, "abc123", 7);
  EXPECT_EQ(7, bytes);

  lseek(nbd_fd, 0, SEEK_SET);

  char buf[7];
  bytes = read(nbd_fd, &buf, 7);
  EXPECT_EQ(7, bytes);
  EXPECT_STREQ("abc123", buf);
}

}

