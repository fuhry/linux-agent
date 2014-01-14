#include "block_device/block_device_factory.h"
#include "block_device/ext_mountable_block_device.h"
#include "block_device/nbd_block_device.h"
#include "block_device/nbd_server.h"
#include "block_device/xfs_mountable_block_device.h"
#include "test/loop_device.h"

#include <gtest/gtest.h>
#include <glog/logging.h>

namespace {

static const uint16_t LOCAL_TEST_PORT = 11235;
static const char LOCAL_TEST_HOST[] = "localhost";

using ::datto_linux_client_test::LoopDevice;
using ::datto_linux_client::BlockDeviceException;
using ::datto_linux_client::BlockDeviceFactory;
using ::datto_linux_client::NbdServer;
using ::datto_linux_client::ExtMountableBlockDevice;
using ::datto_linux_client::XfsMountableBlockDevice;
using ::datto_linux_client::NbdBlockDevice;

TEST(BlockDeviceFactoryTest, Constructor) {
  BlockDeviceFactory fact;
}

TEST(BlockDeviceFactoryTest, ReturnsExt3) {
  BlockDeviceFactory fact;
  LoopDevice loop_dev;

  loop_dev.FormatAsExt3();

  auto loop_block_dev = fact.CreateMountableBlockDevice(loop_dev.path());

  ASSERT_TRUE((bool)loop_block_dev);
  EXPECT_NE(nullptr, dynamic_cast<ExtMountableBlockDevice*>(loop_block_dev.get()));
}

TEST(BlockDeviceFactoryTest, ReturnsXfs) {
  BlockDeviceFactory fact;
  LoopDevice loop_dev;

  loop_dev.FormatAsXfs();

  auto loop_block_dev = fact.CreateMountableBlockDevice(loop_dev.path());

  ASSERT_TRUE((bool)loop_block_dev);
  EXPECT_NE(nullptr, dynamic_cast<XfsMountableBlockDevice*>(loop_block_dev.get()));
}

TEST(BlockDeviceFactoryTest, ThrowsOnNoFS) {
  BlockDeviceFactory fact;
  LoopDevice loop_dev;

  try {
    auto loop_block_dev = fact.CreateMountableBlockDevice(loop_dev.path());
    FAIL() << "Failed to throw exception";
  }
  catch (const BlockDeviceException &e) {
    // good
  }
}

TEST(BlockDeviceFactoryTest, ReturnsNbd) {
  BlockDeviceFactory fact;
  LoopDevice loop_dev;
  NbdServer nbd_server(loop_dev.path(), LOCAL_TEST_PORT);

  auto nbd_block_dev = fact.CreateRemoteBlockDevice(LOCAL_TEST_HOST,
                                                    LOCAL_TEST_PORT);

  ASSERT_TRUE((bool)nbd_block_dev);
  EXPECT_NE(nullptr, dynamic_cast<NbdBlockDevice*>(nbd_block_dev.get()));
}

TEST(BlockDeviceFactoryTest, ThrowsOnNoRemote) {
  BlockDeviceFactory fact;

  try {
    auto nbd_block_dev = fact.CreateRemoteBlockDevice(LOCAL_TEST_HOST,
                                                      LOCAL_TEST_PORT);
    FAIL() << "Didn't throw on missing block device";
  }
  catch (...) {
    // good
  }
}

} // namespace
