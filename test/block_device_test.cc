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

TEST_F(BlockDeviceTest, Major) {
  BlockDevice bd(loop_device->path());
  int major = ::major(bd.dev_t());
  // 7 is the loop device major
  EXPECT_EQ(7, major);
}

TEST_F(BlockDeviceTest, BlockRead) {
  BlockDevice bd(loop_device->path());
  system(("echo -n 'abc123' >> " + loop_device->path()).c_str());

  char buf[4096];

  bd.Read(0, buf, 4096);

  ASSERT_STREQ(buf, "abc123");

  // I'm sure there is a better way to do this
  for (int i = 7; i < 4096; i++) {
    ASSERT_EQ('\0', buf[i]);
  }
}

TEST_F(BlockDeviceTest, BlockWrite) {
  BlockDevice bd(loop_device->path());

  char buf[4096] = "abc123\0";

  bd.Write(0, buf, 4096);
  bd.Read(0, buf, 4096);
  ASSERT_STREQ(buf, "abc123");
}

TEST_F(BlockDeviceTest, BlockRawRead) {
  BlockDevice bd(loop_device->path());

  void *rawbuf;
  if (posix_memalign(&rawbuf, 512, 512)) {
    FAIL() << "posix_memalign";
  }
  char pagedbuf[512];

  bd.BindRaw();

  // Write the block, but it should stick in the page cache
  // and not actually hit disk for at least 15 seconds
  system(("echo -n 'abc123' >> " + loop_device->path()).c_str());

  bd.RawRead(0, rawbuf, 512);
  bd.Read(0, pagedbuf, 512);
  // I'm sure there is a better way to do this
  for (int i = 0; i < 512; i++) {
    ASSERT_EQ('\0', ((char *)rawbuf)[i]);
  }

  ASSERT_STREQ(pagedbuf, "abc123");

  free(rawbuf);
}

} // namespace
