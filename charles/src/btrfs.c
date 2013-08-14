/**
	File:        btrfs.c
	Author:      Charles Roydhouse
	Description: btrfs filesystem parsing functions
*/

#include "btrfs.h"   /* Local */
#include <stdbool.h> /* Standard */
#include <error.h>   /* GNU */
#include <ctree.h>   /* Filesystem */

#define BTRFS_SUPERBLOCK_LOC 0x10000

int btrfs_has_identifier(int fd) {
	struct btrfs_super_block super;
	
	/* Seek to superblock */
	if(lseek(fd, BTRFS_SUPERBLOCK_LOC, SEEK_SET) < 0) {
		return false;
	}
	
	/* Read superblock into struct */
	if(read(fd, &super, sizeof(super)) < 0) {
		return false;
	}
	
	return super.magic == BTRFS_MAGIC;
}

int btrfs_iter_blocks(const char *dev, int (*callback)(int fd, uint64_t length, uint64_t offset)) {
	int rc = 0;
	struct btrfs_root *root;
	struct btrfs_path *path;
	
	radix_tree_init();
	if(!(root = open_ctree(dev, 0, 0))) {
		error(0, errno, "Unable to open %s as btrfs", dev);
		goto out;
	}	
	path = btrfs_alloc_path();
	
	/* TODO: iterate */

out:
	if(path) {
		btrfs_free_path(path);
	}
	if(root) {
		close_ctree(root);
	}
	return rc;
}
