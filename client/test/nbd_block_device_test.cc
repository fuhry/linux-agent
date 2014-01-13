#include "block_device/nbd_block_device.h"
#include "block_device/nbd_server.h"
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
using ::datto_linux_client::NbdServer;

class NbdBlockDeviceTest : public ::testing::Test {
 public:
  NbdBlockDeviceTest() {
    loop_dev = std::unique_ptr<LoopDevice>(new LoopDevice());
    // Let the loop device settle, might not be needed
    sleep(1);
    nbd_server = std::unique_ptr<NbdServer>(new NbdServer(loop_dev->path(),
                                                          LOCAL_TEST_PORT));

    nbd_block_device = std::unique_ptr<NbdBlockDevice>(
        new NbdBlockDevice(LOCAL_TEST_HOST, LOCAL_TEST_PORT));
  }

  ~NbdBlockDeviceTest() { }

  // Order matters here. Lower defined things will be deconstructed last.
  // This way, the loop device isn't pulled out from under the
  // nbd_block_device
  std::unique_ptr<LoopDevice> loop_dev;
  std::unique_ptr<NbdServer> nbd_server;
  std::unique_ptr<NbdBlockDevice> nbd_block_device;
};

TEST_F(NbdBlockDeviceTest, CanConnect) {
  EXPECT_TRUE(nbd_block_device->IsConnected());
}

TEST_F(NbdBlockDeviceTest, KnowsWhenDisconnected) {
  nbd_block_device->Disconnect();
  EXPECT_FALSE(nbd_block_device->IsConnected());
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

