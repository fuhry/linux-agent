#include <block_device/block_device.h>

#include <fcntl.h>
#include <gtest/gtest.h>
#include <memory>
#include <unistd.h>

#include "test/loop_device.h"

namespace {
using ::datto_linux_client::BlockDevice;
using ::datto_linux_client_test::LoopDevice;

class BlockDeviceTest : public ::testing::Test {
 protected:
  BlockDeviceTest() {
    loop_device = std::unique_ptr<LoopDevice>(new LoopDevice());
  }
  std::unique_ptr<LoopDevice> loop_device;
};

TEST_F(BlockDeviceTest, Constructor) {
  BlockDevice bd(loop_device->path());
}

TEST(BlockDeviceTestNoFixture, BadConstructor) {
  bool did_throw = false;
  try {
    BlockDevice bd("Not a path");
  } catch(const std::runtime_error &e) {
    // Good, this should have thrown an exception
    did_throw = true;
  }
  EXPECT_TRUE(did_throw);
}

TEST_F(BlockDeviceTest, Path) {
  BlockDevice bd(loop_device->path());
  EXPECT_EQ(loop_device->path(), bd.path());
}

TEST_F(BlockDeviceTest, BlockSizeBytes) {
  BlockDevice bd(loop_device->path());
  EXPECT_EQ(loop_device->block_size(), bd.BlockSizeBytes());
}

TEST_F(BlockDeviceTest, Open) {
  BlockDevice bd(loop_device->path());
  int bd_fd = bd.Open();
  EXPECT_GT(bd_fd, 2);
}

TEST_F(BlockDeviceTest, Major) {
  BlockDevice bd(loop_device->path());
  int major = ::major(bd.dev_t());
  // 7 is the loop device major
  EXPECT_EQ(7, major);
}

TEST_F(BlockDeviceTest, DestructorCloses) {
  int bd_fd = -1;

  {
    BlockDevice bd(loop_device->path());
    bd_fd = bd.Open();
  } // destructor called here

  int fcntl_ret = fcntl(bd_fd, F_GETFD);
  int error = errno;

  EXPECT_EQ(EBADF, error);
  EXPECT_EQ(-1, fcntl_ret);
}

} // namespace
