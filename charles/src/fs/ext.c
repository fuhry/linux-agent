/**
	File:        ext.c
	Author:      Charles Roydhouse
	Description: ext filesystem parsing functions
*/

#include "ext.h"
#include "fs.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int ext_parse_superblock(int fd, struct ext_fs *fs) {
	uint32_t shift_amnt;
	union {
		uint16_t u16;
		uint8_t byte[sizeof(uint16_t)];
	} signature;
	
	/* Seek to the superblock's block count field location */
	if(lseek(fd, EXT_SUPERBLOCK_LOC + EXT_BLOCKCOUNT_OFF, SEEK_SET) < 0) {
		return FS_SEEK_ER;
	}
	if(read(fd, &fs->total_blocks, sizeof(fs->total_blocks)) < 0) {
		return FS_READ_ER;
	}
	
	/* Seek to the superblock's block shift field location */
	if(lseek(fd, EXT_SUPERBLOCK_LOC + EXT_BLOCKSHIFT_OFF, SEEK_SET) < 0) {
		return FS_SEEK_ER;
	}
	if(read(fd, &shift_amnt, sizeof(shift_amnt)) < 0) {
		return FS_READ_ER;
	}
	fs->block_size = 0x400 << shift_amnt;
	
	
	/* Seek to the superblock's block shift field location */
	if(lseek(fd, EXT_SUPERBLOCK_LOC + EXT_SIGNATURE_OFF, SEEK_SET) < 0) {
		return FS_SEEK_ER;
	}
	if(read(fd, signature.byte, sizeof(signature)) < 0) {
		return FS_READ_ER;
	}	
	
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	signature.u16 = letobe16(signature.u16);
#endif
	fs->signature = signature.u16;
	
	/* Seek to the superblock's blocks per group field location */
	if(lseek(fd, EXT_SUPERBLOCK_LOC + EXT_BLOCKSPERGROUP_OFF, SEEK_SET) < 0) {
		return FS_SEEK_ER;
	}
	if(read(fd, &fs->blocks_per_group, sizeof(fs->blocks_per_group)) < 0) {
		return FS_READ_ER;
	}
	
	/* Calculate the total groups */
	if(fs->total_blocks)
		fs->total_groups = fs->total_blocks / fs->blocks_per_group;
	
	/* Determine location of first group descriptor */
	//TODO: FIX!
	fs->first_group_desc = /*EXT_SUPERBLOCK_LOC +*/ fs->block_size;
	
	return FS_EXIT_OK;
}



char* ext_group_bitmap(int fd, int offset, struct ext_fs *fs) {
	char *bitmap;
	uint32_t bitmap_location;
	/* Seek to the group's bitmap location field */
	if(lseek(fd, offset, SEEK_SET) < 0) {
		return NULL;
	}
	if(read(fd, &bitmap_location, sizeof(bitmap_location)) < 0) {
		return NULL;
	}

	bitmap_location *= fs->block_size;
	bitmap = (char*) malloc(sizeof(char) * fs->block_size);
	
	/* Seek to the group's bitmap location */
	if(lseek(fd, bitmap_location, SEEK_SET) < 0) {
		return NULL;
	}
	if(read(fd, bitmap, fs->block_size) < 0) {
		return NULL;
	}	
	return bitmap;
}
