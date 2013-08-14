/**
	File:        reiser.c
	Author:      Charles Roydhouse
	Description: reiser filesystem parsing functions
*/


#include "reiser.h"            /* Local */
#include "tools.h"
#include <stdbool.h>           /* Standard */
#include <string.h>
#include <errno.h>
#include <fcntl.h>             /* POSIX */
#include <error.h>             /* GNU */
#include <reiserfs/reiserfs.h> /* Filesystem */

#define REISER_SUPERBLOCK_LOC 0x10000

int reiser_has_identifier(int fd) {
	struct reiserfs_super super;
	
	/* Seek to superblock */
	if(lseek(fd, REISER_SUPERBLOCK_LOC, SEEK_SET) < 0) {
		return false;
	}
	
	/* Read superblock into struct */
	if(read(fd, &super, sizeof(super)) < 0) {
		return false;
	}
	
	return strncmp(super.s_v1.sb_magic, REISERFS_3_6_SUPER_SIGNATURE, strlen(REISERFS_3_6_SUPER_SIGNATURE)) == 0;
}



int reiser_iter_blocks(const char *dev, int (*callback)(int fd, uint64_t length, uint64_t offset)) {
	int rc = 0;
	int block_size;
	off_t seek_amnt;
	dal_t *dal = NULL;
	reiserfs_fs_t *fs = NULL;
	reiserfs_bitmap_t *fs_bitmap;
	reiserfs_tree_t *tree;
	
	int fd = open(dev, O_RDONLY);
	if(fd < 0) { 
		error(0, errno, "Error opening %s", dev);
		goto out;
	}	
	
	if(!(dal = (dal_t*) file_dal_open(dev, DEFAULT_BLOCK_SIZE, O_RDONLY))) {
		error(0, errno, "Couldn't create device abstraction for %s", dev);
		goto out;
	}
	
	if(!(fs = reiserfs_fs_open(dal, dal))) {
		error(0, errno, "Unable to open %s as reiserfs", dev);
		goto out;
	}
	
	block_size = fs->super->s_v1.sb_block_size;

	tree = reiserfs_fs_tree(fs);
	fs_bitmap = tree->fs->bitmap;
	
	for(int i = 0; i < fs->super->s_v1.sb_block_count; ++i) {
		if(reiserfs_tools_test_bit(i, fs_bitmap->bm_map)) {
			seek_amnt = /*REISER_SUPERBLOCK_LOC + REISER_SUPERBLOCK_SIZ + */(i * block_size);
			if(lseek(fd, seek_amnt, SEEK_SET) < 0) {
				error(0, errno, "Error seeking %s (Block: %d)", dev, i);
				goto out;
			}
			callback(fd, block_size, seek_amnt);
			rc += block_size;
		}
	}

out:
	if(fd >= 0) {
		close(fd);
	}
	if(fs) {
		reiserfs_fs_close(fs);
	}
	if(dal) {
		file_dal_close(dal);
	}
	return rc;
}
