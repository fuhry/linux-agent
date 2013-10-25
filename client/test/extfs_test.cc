#include <glog/logging.h>
#include <gtest/gtest.h>
#include "fs_parsing/ext_mountable_block_device.h"
#include "block_device/mountable_block_device.h"
#include "unsynced_sector_tracker/sector_set.h"
#include <memory>

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

class EXTFSTest : public ::testing::Test {
 protected:
  EXTFSTest() {
    decompress_image("test/data/filesystems/ext2fs.img.xz");
    mount_device("/dev/loop0", "test/data/filesystems/ext2fs.img");
  }

  ~EXTFSTest() {
    unmount_device("/dev/loop0");
  }
};

TEST_F(EXTFSTest, GetInUseSectorsTest) {
  EXTFSTest test();
  ExtMountableBlockDevice block_dev("/dev/loop0");
  EXPECT_EQ(block_dev.GetInUseSectors(), parse_file("test/data/filesystems/xfs.map"));
}

} // namespace
