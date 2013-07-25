/**
	File:        xfs.c
	Author:      Charles Roydhouse
	Description: xfs filesystem parsing functions
*/

#include "xfs.h"
#include "fs.h"

int xfs_parse_information(int fd, struct xfs_fs *fs) {
	union {
		uint32_t u32;
		uint8_t byte[sizeof(uint32_t)];
	} signature;
	
	/** Seek to the superblock's start (which is also the signature location) */
	if(lseek(fd, XFS_SUPERBLOCK_LOC + XFS_SIGNATURE_OFF, SEEK_SET) < 0) {
		return FS_SEEK_ER;
	}

	if(read(fd, signature.byte, sizeof(signature)) < 0) {
		return FS_READ_ER;
	}
	
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	signature.u32 = letobe32(signature.u32);
#endif
	
	fs->signature = signature.u32;
	return FS_EXIT_OK;
}
