#include "block_device/block_device_factory.h"
#include "block_device/ext_mountable_block_device.h"
#include "block_device/nbd_block_device.h"
#include "block_device/nbd_server.h"
#include "block_device/xfs_mountable_block_device.h"
#include "test/loop_device.h"

#include <gtest/gtest.h>
#include <glog/logging.h>
#include <stdlib.h>

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

// Might want to move this to loop_device.cc
std::string get_uuid(std::string path) {
  std::string uuid;

  int pipefd[2];
  if (pipe(pipefd) == -1) {
    PLOG(ERROR) << "pipe failed";
    throw std::runtime_error("pipe failed");
  }
  int fork_ret = fork();

  if (fork_ret == -1) {
    PLOG(ERROR) << "fork failed";
    throw std::runtime_error("fork failed");
  } else if (fork_ret == 0) {
    // child
    // close unused read end
    close(pipefd[0]);
    // attach stdout to pipe write end
    dup2(pipefd[1], 1);
    close(pipefd[1]);

    execlp("blkid", "blkid", "-p", "-sUUID", "-ovalue", path.c_str(), NULL);
    PLOG(ERROR) << "execlp failed";
    throw std::runtime_error("execlp failed");
  } else {
    // parent
    // close unused write end
    close(pipefd[1]);
    // read from pipe write end
    char out_buf[50];
    int bytes_read;
    if ((bytes_read = read(pipefd[0], out_buf, 50)) == -1) {
      PLOG(ERROR) << "read failed";
      throw std::runtime_error("read failed");
    }
    close(pipefd[0]);

    // Trim newline
    uuid = std::string(out_buf, bytes_read - 1);
  }

  return uuid;
}

TEST(BlockDeviceFactoryTest, Constructor) {
  BlockDeviceFactory fact;
}

TEST(BlockDeviceFactoryTest, ReturnsExt3) {
  BlockDeviceFactory fact;
  LoopDevice loop_dev;

  loop_dev.FormatAsExt3();
  // Wait for udev to make /dev/disk/by-uuid entry
  sleep(1);

  std::string uuid = get_uuid(loop_dev.path());
  auto loop_block_dev = fact.CreateMountableBlockDevice(uuid);

  ASSERT_TRUE((bool)loop_block_dev);
  EXPECT_NE(nullptr,
            dynamic_cast<ExtMountableBlockDevice*>(loop_block_dev.get()));
}

TEST(BlockDeviceFactoryTest, ReturnsXfs) {
  BlockDeviceFactory fact;
  LoopDevice loop_dev;

  loop_dev.FormatAsXfs();
  // Wait for udev to make /dev/disk/by-uuid entry
  sleep(1);

  std::string uuid = get_uuid(loop_dev.path());
  auto loop_block_dev = fact.CreateMountableBlockDevice(uuid);

  ASSERT_TRUE((bool)loop_block_dev);
  EXPECT_NE(nullptr,
            dynamic_cast<XfsMountableBlockDevice*>(loop_block_dev.get()));
}

TEST(BlockDeviceFactoryTest, ThrowsOnBadUUID) {
  BlockDeviceFactory fact;
  // Random UUID that doesn't correspond to any FS
  std::string nonfs_uuid = "50ebcad6-421b-43af-b4ad-68481295b332";

  try {
    auto loop_block_dev = fact.CreateMountableBlockDevice(nonfs_uuid);
    FAIL() << "Failed to throw exception";
  }
  catch (const BlockDeviceException &e) {
    // good
  }
}

TEST(BlockDeviceFactoryTest, ReturnsNbd) {
  BlockDeviceFactory fact;
  LoopDevice loop_dev;
  NbdServer nbd_server(loop_dev.path());

  auto nbd_block_dev = fact.CreateRemoteBlockDevice(LOCAL_TEST_HOST,
                                                    nbd_server.port());

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
