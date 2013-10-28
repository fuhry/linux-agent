#include "fs_parsing/ext_mountable_block_device.h"

#include <gtest/gtest.h>
#include <glog/logging.h>

#include "block_device/mountable_block_device.h"
#include "unsynced_sector_tracker/sector_set.h"
#include "unsynced_sector_tracker/sector_interval.h"
#include "test/loop_device.h"

#include <memory>
#include <fstream>
#include <sstream>

namespace {

using datto_linux_client::ExtMountableBlockDevice;
using datto_linux_client::SectorSet;
using datto_linux_client::SectorInterval;
using datto_linux_client_test::LoopDevice;

void decompress_image(std::string compressed_path,
                      std::string destination_path) {

  if (destination_path.find(" ") != std::string::npos) {
    FAIL() << "destination_path has a space, this function will fail";
  }

  if (system(("unxz --keep --stdout " +
                compressed_path + " > " + destination_path).c_str())) {
    FAIL() << "Error while decompressing image";
  }
}

SectorSet map_to_sector_set(std::string map_path, uint64_t block_size) {
  std::ifstream map_stream(map_path);
  std::string line;

  SectorSet sector_set;

  while (std::getline(map_stream, line)) {
    std::stringstream line_stream(line);

    uint64_t start_value;
    uint64_t length;

    line_stream >> std::hex >> start_value;
    line_stream >> std::hex >> length;

    start_value /= block_size;
    length /= block_size;

    // Add 1 as the end bound is exclusive
    sector_set.add(SectorInterval(start_value, start_value + length + 1));
  }

  return sector_set;
}


class ExtFSTest : public ::testing::Test {
 protected:
  ExtFSTest() {
  }

  ~ExtFSTest() {
  }
};

TEST_F(ExtFSTest, Construct) {
  std::string test_out = "/tmp/ext2fs.img";
  decompress_image("test/data/filesystems/ext2fs.img.xz", test_out);

  LoopDevice loop_dev(test_out);

  ExtMountableBlockDevice bdev(loop_dev.path());

  SectorSet expected = map_to_sector_set("test/data/filesystems/ext2fs.map",
                                         1024);
  SectorSet actual = *bdev.GetInUseSectors();

  for (auto interval : actual) {
    LOG(INFO) << "a " << interval.lower() << " : " << interval.upper();
  }

  for (auto interval : expected) {
    LOG(INFO) << "e " << interval.lower() << " : " << interval.upper();
  }

  EXPECT_TRUE(expected == actual);

}

} // namespace
