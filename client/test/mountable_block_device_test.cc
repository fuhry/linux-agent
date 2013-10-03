#include "block_device/mountable_block_device.h"

#include <fcntl.h>
#include <gtest/gtest.h>
#include <memory>
#include <unistd.h>

#include "test/loop_device.h"
#include "unsynced_sector_tracker/sector_set.h"

namespace {
using ::datto_linux_client::MountableBlockDevice;
using ::datto_linux_client::SectorSet;
using ::datto_linux_client_test::LoopDevice;

class MountableBlockDeviceTest : public ::testing::Test {
 protected:
  MountableBlockDeviceTest() {
    loop_device = std::unique_ptr<LoopDevice>(new LoopDevice());
    // Make it mountable by putting a FS on it
    if (system(("mkfs.ext2 " + loop_device->path() +
                " 2>&1 1>/dev/null").c_str())) {
      throw std::runtime_error("error creating fs");
    }

    temp_dir = "/tmp/test_mount";
    if (system(("mkdir -p " + temp_dir).c_str())) {
      throw std::runtime_error("error creating test_mount directory");
    }
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

void mount_device(std::string path, std::string mount_point) {
  if (system(("mount -t ext2 " + path + " " + mount_point).c_str())) {
    FAIL() << "Error while mounting device";
  }
}

void unmount_device(std::string path) {
  if (system(("umount " + path).c_str())) {
    FAIL() << "Error unmounting device";
  }
}
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

} // namespace
