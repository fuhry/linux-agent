#include "fs_parsing/ext_mountable_block_device.h"

#include "fs_parsing/ext_file_system.h"
#include "fs_parsing/ext_error_table-inl.h"

#include "block_device/block_device_exception.h"

#include "unsynced_sector_tracker/sector_interval.h"

#include <fcntl.h>
#include <unistd.h>

#include <glog/logging.h>

#include "fs_parsing/tools.h"

namespace datto_linux_client {

ExtMountableBlockDevice::ExtMountableBlockDevice(std::string a_block_path)
    : MountableBlockDevice(a_block_path) { }

std::unique_ptr<const SectorSet> ExtMountableBlockDevice::GetInUseSectors() {
  std::unique_ptr<SectorSet> sectors(new SectorSet());
  ExtErrorTable error_table;
  ExtFileSystem ext_fs(BlockDevice::block_path(), error_table);

  int block_size;
  int bitmap_size;
  off_t cur_group_block_offset;
  off_t cur_block_offset;
  off_t seek_amnt;

  int rc;

  block_size = 0x400 << ext_fs.fs()->super->s_log_block_size;
  bitmap_size = my_roundup(ext_fs.fs()->super->s_blocks_per_group, 8) / 8;

  std::unique_ptr<char[]> block_bitmap(new char[bitmap_size]);

  // First sector is always included
  sectors->insert(SectorInterval(0, 512));

  for (uint64_t i = 0; i < ext_fs.fs()->group_desc_count; ++i) {

    /* Calculate the block offset for the current group */
    cur_group_block_offset = ext_fs.fs()->super->s_first_data_block +
                             (i * ext_fs.fs()->super->s_blocks_per_group);

    ext2fs_get_block_bitmap_range(ext_fs.fs()->block_map,
                                  cur_group_block_offset,
                                  ext_fs.fs()->super->s_blocks_per_group,
                                  block_bitmap.get());

    /* Break if we have reached the end of this group (s_blocks_per_group)
     * or the end of the file system (s_blocks_count) */
    for (uint64_t j = 0;
         (j < ext_fs.fs()->super->s_blocks_per_group &&
            j + cur_group_block_offset < ext_fs.fs()->super->s_blocks_count);
         ++j) {

      /* If the bit is set then the block is allocated. */
      if (ext2fs_test_bit(j, block_bitmap.get())) {
        cur_block_offset = cur_group_block_offset + j;
        seek_amnt = block_size * cur_block_offset;

        if (lseek(ext_fs.dev_fd(), seek_amnt, SEEK_SET) < 0) {
          PLOG(ERROR) << "Failure during seek";
          throw BlockDeviceException("Unable to seek device");
        }

        uint64_t sector_location = seek_amnt/block_size;
        // TODO Use constant for 512
        uint64_t blocks_per_sector = block_size / 512;

        sectors->insert(SectorInterval(sector_location,
                                       sector_location + blocks_per_sector));
        rc += block_size;
      }
    }
  }

  return std::move(sectors);
}

} // namespace datto_linux_client
