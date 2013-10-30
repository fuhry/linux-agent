#include "block_device/mountable_block_device.h"

#include <atomic>
#include <fcntl.h>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <unistd.h>

#include "test/loop_device.h"
#include "unsynced_sector_store/sector_set.h"

namespace {
using ::datto_linux_client::MountableBlockDevice;
using ::datto_linux_client::SectorSet;
using ::datto_linux_client_test::LoopDevice;

void mount_device(std::string path, std::string mount_point) {
  if (system(("mount -t ext3 " + path + " " + mount_point).c_str())) {
    FAIL() << "Error while mounting device";
  }
}

void unmount_device(std::string path) {
  int suppress_warning = system(("umount " + path + " 2>/dev/null").c_str());
  (void) suppress_warning;
}

class MountableBlockDeviceTest : public ::testing::Test {
 protected:
  MountableBlockDeviceTest() {
    loop_device = std::unique_ptr<LoopDevice>(new LoopDevice());
    // Make it mountable by putting a FS on it
    loop_device->FormatAsExt3();

    temp_dir = "/tmp/test_mount";
    if (system(("mkdir -p " + temp_dir).c_str())) {
      throw std::runtime_error("error creating test_mount directory");
    }
  }

  ~MountableBlockDeviceTest() {
    unmount_device(loop_device->path());
  }

  std::unique_ptr<LoopDevice> loop_device;
  std::string temp_dir;
};

class MockMountableBlockDevice : public MountableBlockDevice {
 public:
  MockMountableBlockDevice(std::string a_block_path)
    : MountableBlockDevice(a_block_path) { }

  std::unique_ptr<const SectorSet> GetInUseSectors();
};

std::unique_ptr<const SectorSet> MockMountableBlockDevice::GetInUseSectors() {
  return nullptr;
}

TEST_F(MountableBlockDeviceTest, Constructor) {
  MockMountableBlockDevice bd(loop_device->path());
}

TEST_F(MountableBlockDeviceTest, IsMounted) {
  MockMountableBlockDevice bd(loop_device->path());

  EXPECT_FALSE(bd.IsMounted());
  mount_device(loop_device->path(), temp_dir);
  EXPECT_TRUE(bd.IsMounted());
  unmount_device(loop_device->path());
}

TEST_F(MountableBlockDeviceTest, GetMountPoint) {
  MockMountableBlockDevice bd(loop_device->path());

  mount_device(loop_device->path(), temp_dir);
  EXPECT_EQ(temp_dir, bd.GetMountPoint());
  unmount_device(loop_device->path());
}

TEST_F(MountableBlockDeviceTest, OpenMount) {
  mount_device(loop_device->path(), temp_dir);

  // Scope this so things are cleaned up before unmounted
  {
    MockMountableBlockDevice bd(loop_device->path());
    int mount_fd = bd.OpenMount();

    EXPECT_GE(mount_fd, 0);
  }

  unmount_device(loop_device->path());
}

TEST_F(MountableBlockDeviceTest, CloseMount) {
  mount_device(loop_device->path(), temp_dir);

  // Scope this so things are cleaned up before unmounted
  {
    MockMountableBlockDevice bd(loop_device->path());
    int mount_fd = bd.OpenMount();
    bd.CloseMount();

    int fcntl_ret = fcntl(mount_fd, F_GETFD);

    // Should have been an error as we closed it
    EXPECT_EQ(-1, fcntl_ret);
  }

  unmount_device(loop_device->path());
}

TEST_F(MountableBlockDeviceTest, FreezeAndThaw) {
  MockMountableBlockDevice bd(loop_device->path());

  mount_device(loop_device->path(), temp_dir);

  // Freeze the device
  bd.Freeze();

  std::atomic<bool> is_done(false);
  // Try to write to it in another thread
  std::thread write_thread([&]() {
      int suppress_warning = system(("touch " + temp_dir + "/a_file").c_str());
      (void) suppress_warning;
      is_done = true;
  });

  // make sure the thread starts execution
  sleep(1);
  ASSERT_TRUE(write_thread.joinable());

  write_thread.detach();
  // Make sure that thread is blocked
  EXPECT_FALSE(is_done);

  // Thaw
  bd.Thaw();

  // 5 seconds should be more than long enough for the write_thread to schedule
  for (int i = 0; i < 5; ++i) {
    if (is_done) {
      break;
    }
    sleep(1);
  }
  // Make sure write went through
  EXPECT_TRUE(is_done);

  unmount_device(loop_device->path());
}

} // namespace
