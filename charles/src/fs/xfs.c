/**
	File:        xfs.c
	Author:      Charles Roydhouse
	Description: xfs filesystem parsing functions
*/

#include "xfs.h"
#include "fs.h"
#include <xfs/xfs.h>
#include <stdbool.h>
#include <stdio.h>

int xfs_has_identifier(int fd) {
	return 1;
/*
	uint32_t signature;
	xfs_sb_t super;
	
	// Seek to superblock 
	if(lseek(fd, XFS_SUPERBLOCK_LOC, SEEK_SET) < 0) {
		return false;
	}
	
	// Read superblock into struct
	if(read(fd, &super, sizeof(super)) < 0) {
		return false;
	}
	
	// Switch byte-order to big endian if needed 
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	signature = letobe32(super.sb_magicnum);
#endif

	return signature == XFS_SIGNATURE;
*/
}


int xfs_iter_blocks(const char *dev, int (*callback)(int fd, uint64_t length, uint64_t offset)) {
	return 1;
/*
	int rc = FS_EXIT_OK;
	int block_size;
	off_t seek_amnt;
	xfs_sb_t super;
		
	int fd = open(dev, O_RDONLY);
	if(fd < 0) { 
		error(0, errno, "Error opening %s", dev);
		goto out;
	}
	
	// Seek to superblock
	if(lseek(fd, XFS_SUPERBLOCK_LOC, SEEK_SET) < 0) {
		return false;
	}
	
	// Read superblock into struct
	if(read(fd, &super, sizeof(super)) < 0) {
		return false;
	}
	
	block_size = sb_blocksize;
	
	
	
out:
	if(!(fd < 0)) {
		close(fd);
	}
	return rc;
*/
}
