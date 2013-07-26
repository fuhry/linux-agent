/**
	File:        xfs.c
	Author:      Charles Roydhouse
	Description: xfs filesystem parsing functions
*/

#include "xfs.h"
#include "fs.h"
#include "../include/xfs.h"
#include <stdbool.h>
#include <stdio.h>

int xfs_has_identifier(int fd) {
	uint32_t signature;
	xfs_sb_t super;
	
	/* Seek to superblock */
	if(lseek(fd, XFS_SUPERBLOCK_LOC, SEEK_SET) < 0) {
		return false;
	}
	
	/* Read superblock into struct */
	if(read(fd, &super, sizeof(super)) < 0) {
		return false;
	}
	
	/* Switch byte-order to big endian if needed */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	signature = letobe32(super.sb_magicnum);
#endif

	return signature == XFS_SIGNATURE;
}


int xfs_iter_blocks(const char *dev, int (*callback)(int fd, uint64_t length, uint64_t offset)) {
	//TODO: learn
	return true;
}
