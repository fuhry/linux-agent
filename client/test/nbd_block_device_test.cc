#include "remote_block_device/nbd_block_device.h"

#include <memory>
#include <gtest/gtest.h>

using ::datto_linux_client::NbdBlockDevice;

namespace {

class NbdBlockDeviceTest : public ::testing::Test {
 public:
 protected:
  NbdBlockDeviceTest() {
    // Setup 'remote' block device
  }

  ~NbdBlockDeviceTest() {
    // Teardown 'remote' block device
  }

  std::unique_ptr<NbdBlockDevice> nbd_block_device;
};

TEST_F(NbdBlockDeviceTest, CanConnect) {
  EXPECT_TRUE(nbd_block_device->IsConnected());
}

}

