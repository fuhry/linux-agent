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
	rc = ext_parse_information(fd, &fs);
	return !rc && fs.signature == EXT_SIGNATURE;
}



int xfs_identify(int fd) {
	int rc;
	struct xfs_fs fs;
	rc = xfs_parse_information(fd, &fs);
	return !rc && fs.signature == XFS_SIGNATURE;
}



int reiserfs_identify(int fd) {
	int rc;
	struct reiser_fs fs;
	rc = reiser_parse_information(fd, &fs);
	return !rc && strncmp(fs.signature, REISER_SIGNATURE, REISER_SIGNATURE_LEN) == 0;
}



int ext2_iter_blocks(int fd, int (*callback)(int fd, uint64_t length)) {
	int i, rc;
	struct ext_fs fs;
	rc = ext_parse_information(fd, &fs);
	if(rc) {
		fprintf(stderr, "Could not parse ext family filesystem error code %d\n", rc);
		return FS_EXIT_ER_UNKOWN;
	}
		
	for(i = 0; i < fs.total_groups; ++i) {
			
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
		fprintf(stderr, "Could not open device `%s`\n", dev);
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
	rc = (close(fd) == 0) && rc;
	return rc;
}



int fs_iter_blocks(const char *dev, const int FS_TYPE, int (*callback)(int fd, uint64_t length)) {
	int fd = open(dev, O_RDONLY);
	int rc = 0;
	if(fd < 0) { 
		fprintf(stderr, "Could not open device `%s`\n", dev);
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
	rc = close(fd) ^ rc; /* bitwise or: 0x0 ^ 0x0 = 0x0, anything else not 0x00 */
	return rc;
}
