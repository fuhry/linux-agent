/**
	File:        ext.c
	Author:      Charles Roydhouse
	             Nick Garvey
	Description: ext filesystem parsing functions
*/

#include "ext.h"
#include "fs.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <ext2fs/ext2_err.h>
#include <ext2fs/ext2_io.h>
#include <ext2fs/ext2fs.h>

#define EXT_SUPERBLOCK_LOC 0x400
#define EXT_SUPERBLOCK_SIZ 0x400

int ext_has_identifier(int fd) {
	uint16_t signature;
	struct ext2_super_block super;

	/* Seek to superblock */
	if(lseek(fd, EXT_SUPERBLOCK_LOC, SEEK_SET) < 0) {
		return false;
	}
	
	/* Read superblock into struct */
	if(read(fd, &super, EXT_SUPERBLOCK_SIZ) < 0) {
		return false;
	}
	
	/* Switch byte-order to big endian if needed */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	signature = letobe16(super.s_magic);
#endif
	return signature == EXT_SIGNATURE;
}



int ext_iter_blocks(const char *dev, int (*callback)(int fd, uint64_t length, uint64_t offset)) {
	int rc = 0;
	int block_size, bitmap_size;
	ext2_filsys fs = NULL;
	void *block_bitmap = NULL;
	off_t cur_group_block_offset;
	off_t cur_block_offset;
	off_t seek_amnt;
		
	int fd = open(dev, O_RDONLY);
	if(fd < 0) { 
		error(0, errno, "Error opening %s", dev);
		goto out;
	}	
	
	add_error_table(&et_ext2_error_table);
	if((rc = ext2fs_open(dev, 0, 0, 0, unix_io_manager, &fs))) {
		error(0, errno, "Unable to open %s as extfs - %s", dev, error_message(rc));
		goto out;
	}
	
	if((rc = ext2fs_read_bitmaps(fs))) {
		error(0, errno, "Unable to read %s bitmap - %s", dev, error_message(rc));
		goto out;
	}
	
	block_size = 0x400 << fs->super->s_log_block_size;
	bitmap_size = roundup(fs->super->s_blocks_per_group, 8) / 8;
	block_bitmap = malloc(bitmap_size);
	
	for(int i = 0; i < fs->group_desc_count; ++i) {

		/* Calculate the block offset for the current group */
		cur_group_block_offset = fs->super->s_first_data_block + (i * fs->super->s_blocks_per_group);

		ext2fs_get_block_bitmap_range(fs->block_map, cur_group_block_offset, fs->super->s_blocks_per_group, block_bitmap);

		/* Break if we have reached the end of this group (s_blocks_per_group)
		 * or the end of the file system (s_blocks_count) */
		for(int j = 0; j < fs->super->s_blocks_per_group && j + cur_group_block_offset < fs->super->s_blocks_count; ++j) {

			/* If the bit is set then the block is allocated. */
			if(ext2fs_test_bit(j, block_bitmap)) {
				cur_block_offset = cur_group_block_offset + j;
				seek_amnt = block_size * cur_block_offset;

				if(lseek(fd, seek_amnt, SEEK_SET) < 0) {
					error(0, errno, "Error seeking %s (Block: %ld)", dev, (long) cur_block_offset);
					goto out;
				}
				callback(fd, block_size, seek_amnt);
				rc += block_size;
			}
		}
	}
	
out:
	remove_error_table(&et_ext2_error_table);
	
	if(fs) {
		ext2fs_close(fs);
	}
	if(!(fd < 0)) {
		close(fd);
	}
	if(block_bitmap) {
		free(block_bitmap);
	}
	return rc;
}
