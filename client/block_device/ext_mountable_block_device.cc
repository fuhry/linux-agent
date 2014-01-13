#include "block_device/ext_mountable_block_device.h"

#include <fcntl.h>
#include <unistd.h>

#include <glog/logging.h>

#include "block_device/ext_file_system.h"
#include "block_device/ext_error_table-inl.h"
#include "block_device/block_device_exception.h"
#include "unsynced_sector_manager/sector_interval.h"
#include "block_device/tools.h"

namespace datto_linux_client {

ExtMountableBlockDevice::ExtMountableBlockDevice(std::string a_path)
    : MountableBlockDevice(a_path) { }

std::unique_ptr<const SectorSet> ExtMountableBlockDevice::GetInUseSectors() {
  std::unique_ptr<SectorSet> sectors(new SectorSet());
  ExtErrorTable error_table;
  ExtFileSystem ext_fs(BlockDevice::path(), error_table);

  uint32_t blocks_per_group = ext_fs.super()->s_blocks_per_group;
  uint32_t block_size = 0x400 << ext_fs.super()->s_log_block_size;
  uint64_t sectors_per_block = block_size / 512;
  uint32_t bitmap_size = my_roundup(blocks_per_group, 8) / 8;

  std::unique_ptr<char[]> block_bitmap(new char[bitmap_size]);

  // First two sectors (sector 0, 1) are always included as it isn't managed by
  // the fs
  sectors->insert(SectorInterval(0, 2));

  // Iterates over every group in the file system
  for (uint64_t i = 0; i < ext_fs.group_desc_count(); ++i) {
    DLOG(INFO) << "Checking ext group #" << i;

    // Calculate the block offset for the current group
    off_t cur_group_block_offset = ext_fs.super()->s_first_data_block +
                                   (i * blocks_per_group);

    // Load the block bitmap for this group
    ext2fs_get_block_bitmap_range(ext_fs.block_map(),
                                  cur_group_block_offset,
                                  blocks_per_group,
                                  block_bitmap.get());

    // Iterate over every block assigned to this group
    // Break if we have reached the end of this group (locks_per_group)
    // or the end of the file system (s_blocks_count)
    for (uint64_t j = 0;
         (j < blocks_per_group &&
            (j + cur_group_block_offset < ext_fs.super()->s_blocks_count));
         ++j) {

      // If the bit is set then the block is allocated.
      if (ext2fs_test_bit(j, block_bitmap.get())) {
        off_t cur_block_offset = cur_group_block_offset + j;

        // Calculate the sector for the current block
        uint64_t sector_location = sectors_per_block * cur_block_offset;

        // Insert an interval starting at the sector for the current block
        // and ending at the last sector for the current block
        // Remember that the SectorIntervals are [x, y) where x is 
        // inclusive and y is exclusive.
        sectors->insert(SectorInterval(sector_location,
                                       sector_location + sectors_per_block));
      }
    }
  }

  LOG(INFO) << "ext file system is size "
            << boost::icl::cardinality(*sectors) * 512;

  return std::move(sectors);
}

} // namespace datto_linux_client
