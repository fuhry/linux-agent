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
static const char LOCAL_TEST_NBD_PATH[] = "/dev/nbd1";

namespace {

using ::datto_linux_client_test::LoopDevice;

class NbdBlockDeviceTest : public ::testing::Test {
 public:
 protected:
  class NbdServer {
   public:
    NbdServer(std::string file_to_serve) {
      // null_fd is for redirecting below
      int null_fd = open("/dev/null", O_RDWR);
      pid_t fork_ret = fork();
      switch (fork_ret) {
        case -1:
          PLOG(ERROR) << "fork";
          throw std::runtime_error("fork failed");
          break;
        case 0:
          // This is the same as 2>&1 1>/dev/null
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
          break;
        default:
          nbd_server_pid = fork_ret;
          // Let the nbd_server start
          sleep(1);
          break;
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
        new NbdBlockDevice(LOCAL_TEST_HOST, LOCAL_TEST_PORT,
                           LOCAL_TEST_NBD_PATH));
  }

  ~NbdBlockDeviceTest() {
    nbd_block_device->Disconnect();
  }

  std::unique_ptr<NbdBlockDevice> nbd_block_device;
  std::unique_ptr<NbdServer> nbd_server;
  std::unique_ptr<LoopDevice> loop_dev;
};

TEST_F(NbdBlockDeviceTest, CanConnect) {
  EXPECT_TRUE(nbd_block_device->IsConnected());
}

TEST(NbdBlockDeviceTestNoFixture, CantConnect) {
  try {
    auto nbd_block_device = std::unique_ptr<NbdBlockDevice>(
        new NbdBlockDevice(LOCAL_TEST_HOST, LOCAL_TEST_PORT,
          LOCAL_TEST_NBD_PATH));
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

}

