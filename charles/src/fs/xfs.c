/**
	File:        xfs.c
	Author:      Charles Roydhouse
	Description: xfs filesystem parsing functions
*/

#include "xfs.h"
#include "fs.h"
#include <stdbool.h>

int xfs_has_identifier(int fd) {
	union {
		uint32_t u32;
		uint8_t byte[sizeof(uint32_t)];
	} signature;
	
	/** Seek to the superblock's start (which is also the signature location) */
	if(lseek(fd, XFS_SUPERBLOCK_LOC + XFS_SIGNATURE_OFF, SEEK_SET) < 0) {
		return false;
	}

	if(read(fd, signature.byte, sizeof(signature)) < 0) {
		return false;
	}
	
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	signature.u32 = letobe32(signature.u32);
#endif
	
	return signature.u32 == XFS_SIGNATURE;
}


int xfs_iter_blocks(const char *dev, int (*callback)(int fd, uint64_t length)) {
	return true;
}
