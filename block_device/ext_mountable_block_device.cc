#include "block_device/ext_mountable_block_device.h"

#include <fcntl.h>
#include <unistd.h>

#include <glog/logging.h>

#include "block_device/block_device_exception.h"
#include "block_device/ext_error_table-inl.h"
#include "block_device/ext_file_system.h"
#include "block_device/tools.h"
#include "freeze_helper/freeze_helper.h"
#include "unsynced_sector_manager/sector_interval.h"

namespace datto_linux_client {

ExtMountableBlockDevice::ExtMountableBlockDevice(std::string a_path,
                                                 bool is_ext2)
    : MountableBlockDevice(a_path), is_ext2_(is_ext2) { }

std::shared_ptr<const SectorSet> ExtMountableBlockDevice::GetInUseSectors() {
  std::shared_ptr<SectorSet> sectors(new SectorSet());
  ExtErrorTable error_table;

  FreezeHelper freeze_helper(*this, 2000);

  std::unique_ptr<ExtFileSystem> ext_fs;
  freeze_helper.RunWhileFrozen([&]() {
    ext_fs = std::unique_ptr<ExtFileSystem>(
        new ExtFileSystem(BlockDevice::path(), error_table));
  });

  uint32_t bitmap_size;
  uint32_t block_size;
  uint32_t blocks_per_group;
  uint64_t blocks_count;
  uint64_t group_desc_count;
  uint64_t sectors_per_block;

  freeze_helper.RunWhileFrozen([&]() {
    blocks_per_group = ext_fs->super()->s_blocks_per_group;
    block_size = 0x400 << ext_fs->super()->s_log_block_size;
    sectors_per_block = block_size / 512;
    bitmap_size = my_roundup(blocks_per_group, 8) / 8;
    group_desc_count = ext_fs->group_desc_count();
    blocks_count = ext_fs->super()->s_blocks_count;
  });

  std::unique_ptr<char[]> block_bitmap(new char[bitmap_size]);

  // First two sectors (sector 0, 1) are always included as it isn't managed by
  // the fs
  sectors->insert(SectorInterval(0, 2));


  // Iterates over every group in the file system
  for (uint64_t i = 0; i < group_desc_count; ++i) {
    off_t cur_group_block_offset;
    DLOG(INFO) << "Checking ext group #" << i;
    freeze_helper.RunWhileFrozen([&]() {
      // Calculate the block offset for the current group
      cur_group_block_offset = ext_fs->super()->s_first_data_block +
                                   (i * blocks_per_group);

      // Load the block bitmap for this group
      ext2fs_get_block_bitmap_range(ext_fs->block_map(),
                                    cur_group_block_offset,
                                    blocks_per_group,
                                    block_bitmap.get());
    });


    // Iterate over every block assigned to this group
    // Break if we have reached the end of this group (locks_per_group)
    // or the end of the file system (s_blocks_count)
    for (uint64_t j = 0;
         (j < blocks_per_group &&
            (j + cur_group_block_offset < blocks_count));
         ++j) {
      if (ext2fs_test_bit(j, block_bitmap.get())) {
        // If the bit is set then the block is allocated.
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

  return sectors;
}

void ExtMountableBlockDevice::Freeze() {
  if (!is_ext2_) {
    MountableBlockDevice::Freeze();
  }
}

void ExtMountableBlockDevice::Thaw() {
  if (!is_ext2_) {
    MountableBlockDevice::Thaw();
  }
}

} // namespace datto_linux_client
