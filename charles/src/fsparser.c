/**
	File:        fsparser.c
	Author:      Charles Roydhouse
	Description: Provides implementation for a wrapper over POSIX/Linux for various filesystems
*/

#include "fsparser.h"
#include "fs/ext.h"
#include "fs/xfs.h"
#include "fs/reiser.h"

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
	
#if !(defined TRUE && defined FALSE)
	#define TRUE 1
	#define FALSE 0
#endif

/*
	Developer Note:
		To add new filesystem functionality:
			- Add an entry to the header file giving the new filesystem a unique identifier
			- Add a static declaration that follows the templates already layed out
				- A function that identifies the filesystem
				- A function that iterates over the used blocks and calls the callback on them
			- Define the declaration
				- A function that identifies the filesystem
				- A function that iterates over the used blocks and calls the callback on them
			- Add the new filesystem identifier to the two switches in the lookup table section
			- Kudos if you offload
*/

/*********************************STATIC DECLARATIONS**************************/
static int ext2_identify(int fd);
static int xfs_identify(int fd);
static int reiserfs_identify(int fd);

static int ext2_iter_blocks(int fd, int (*callback)(int fd, uint64_t length));
static int xfs_iter_blocks(int fd, int (*callback)(int fd, uint64_t length));
static int reiserfs_iter_blocks(int fd, int (*callback)(int fd, uint64_t length));

/*******************************FILESYSTEM FUNCTIONALITY***********************/

int ext2_identify(int fd) {
	int rc;
	struct ext_fs fs;
	rc = ext_parse_superblock(fd, &fs);
	if(rc) {
		fprintf(stderr, "Could not parse filesystem at 0x%x error %d\n", fd, rc);
	}
	return !rc && fs.signature == EXT_SIGNATURE;
}



int xfs_identify(int fd) {
	int rc;
	struct xfs_fs fs;
	rc = xfs_parse_superblock(fd, &fs);
	if(rc) {
		fprintf(stderr, "Could not parse filesystem at 0x%x error %d\n", fd, rc);
	}
	return !rc && fs.signature == XFS_SIGNATURE;
}



int reiserfs_identify(int fd) {
	int rc;
	struct reiser_fs fs;
	rc = reiser_parse_superblock(fd, &fs);
	if(rc) {
		fprintf(stderr, "Could not parse filesystem at 0x%x error %d\n", fd, rc);
	}
	return !rc && strncmp(fs.signature, REISER_SIGNATURE, REISER_SIGNATURE_LEN) == 0;
}



int ext2_iter_blocks(int fd, int (*callback)(int fd, uint64_t length)) {
	int i, rc;
	struct ext_fs fs;
	rc = ext_parse_superblock(fd, &fs);
	if(rc) {
		fprintf(stderr, "Could not parse ext family filesystem error code %d\n", rc);
		return FS_EXIT_ER_PARSE;
	}
	
	printf("%x %d %d %ld %d %d\n", fs.signature, fs.total_blocks, fs.total_groups, fs.block_size, fs.blocks_per_group, fs.first_group_desc);
	for(i = 0; i < fs.total_groups; ++i) {
			int cur_group_block_offset = fs.first_group_desc + (fs.blocks_per_group * fs.block_size  * i);
			//TODO: FIX!
			char *bitmap = ext_group_bitmap(fd, cur_group_block_offset, &fs);
			printf("0x"); int j;
			for(j = 0; j < 
			if(bitmap) {
				free(bitmap);
			}
	}
	return FS_EXIT_OK;
}



int xfs_iter_blocks(const int fd, int (*callback)(int fd, uint64_t length)) {
	return FS_EXIT_OK;
}



int reiserfs_iter_blocks(const int fd, int (*callback)(int fd, uint64_t length)) {
	return FS_EXIT_OK;
}

/************************************LOOKUP TABLE*******************************/
int fs_identify(const char *dev, const int FS_TYPE) {
	int fd = open(dev, O_RDONLY);
	int rc = 0;
	if(fd < 0) { 
		return FS_EXIT_ER_OPEN;
	}
	switch(FS_TYPE) {
		case FS_EXT2_T:
			rc =  ext2_identify(fd);
			break;
		
		case FS_XFS_T:
			rc =  xfs_identify(fd);
			break;
					
		case FS_REISERFS_T:
			rc = reiserfs_identify(fd);
			break;
			
		default:
			close(fd);
			return FS_EXIT_ER_ID_UNDEF;
	}	
	if(close(fd)) {
			return FS_EXIT_ER_CLOSE;
	}
	return rc;
}



int fs_iter_blocks(const char *dev, const int FS_TYPE, int (*callback)(int fd, uint64_t length)) {
	int fd = open(dev, O_RDONLY);
	int rc = 0;
	if(fd < 0) { 
		return FS_EXIT_ER_OPEN;
	}	
	switch(FS_TYPE) {
		case FS_EXT2_T:
			rc = ext2_iter_blocks(fd, callback);
			break;
		
		case FS_XFS_T:
			rc = xfs_iter_blocks(fd, callback);
			break;
		
		case FS_REISERFS_T:
			rc = reiserfs_iter_blocks(fd, callback);
			break;
		
		default:
			close(fd);
			return FS_EXIT_ER_ITER_UNDEF;
	}
	if(close(fd)) {
			return FS_EXIT_ER_CLOSE;
	}
	return rc;
}
