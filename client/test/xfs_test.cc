#include <glog/logging.h>
#include <gtest/gtest.h>
#include "fs_parsing/xfs_mountable_block_device.h"

namespace {

void decompress_image(std::string compressed_image) {
  if (system(("xz -kd " + compressed_image).c_str())) {
    FAIL() << "Error while decompressing image";
  }
}

void mount_device(std::string mount_point, std::string image) {
  if (system(("losetup " + mount_point + " " + image).c_str())) {
    FAIL() << "Error while mounting image";
  }
}

void unmount_device(std::string mount_point) {
  int suppress_warning = system(("losetup -d " + mount_point + " 2>/dev/null").c_str());
  (void) suppress_warning;
}

std::unique_ptr<const SectorSet> parse_file(std::string file) {
}

class XFSTest : public ::testing::Test {
 protected:
  XFSTest() {
    decompress_image("test/data/filesystems/xfs.img.xz");
    mount_device("/dev/loop0", "test/data/filesystems/xfs.img");
  }

  ~XFSTest() {
    unmount_device("/dev/loop0");
  }
};

TEST_F(EXTFSTest) {
  ExtMountableBlockDevice block_dev("/dev/loop0");
  std::unique_ptr<const SectorSet> block_dev_data = block_dev.GetInUseSectors();
  std::unique_ptr<const SectorSet> file_data = parse_file("test/data/filesystems/xfs.map");
  EXPECT_EQ(block_dev_data, file_data);
}

} // namespace
