/**
	File:        fsparser.c
	Author:      Charles Roydhouse
	Description: Provides implementation for a wrapper over POSIX/Linux for various filesystems
*/

#include "fsparser.h"
#include <fcntl.h>
#include <unistd.h>

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
static int ext2_identify(const char *dev);
static int xfs_identify(const char *dev);
static int reiserfs_identify(const char *dev);

static int ext2_iter_blocks(const char *dev, int (*callback)(void* buffer, uint64_t length));
static int xfs_iter_blocks(const char *dev, int (*callback)(void* buffer, uint64_t length));
static int reiserfs_iter_blocks(const char *dev, int (*callback)(void* buffer, uint64_t length));

/*******************************FILESYSTEM FUNCTIONALITY***********************/

int ext2_identify(const char *dev) {
	int fd = open(dev, O_RDONLY);
	uint16_t signature = 0x00;
	int rc = 0;
	
	if(fd < 0) return FALSE;
	if(lseek(fd, 0x6c0, SEEK_SET) < 0) {
		close(fd);
		return FALSE;
	}

	if(read(fd, &signature, sizeof(signature)) < 0) {
		close(fd);
		return FALSE;
	}
	
	rc = close(fd);
	rc = (signature == 0xef53) && (rc == 0);
	return rc;
}

int xfs_identify(const char *dev) {
	return FALSE;
}

int reiserfs_identify(const char *dev) {
	return FALSE;
}

int ext2_iter_blocks(const char *dev, int (*callback)(void* buffer, uint64_t length)) {
	return FS_EXIT_OK;
}

int xfs_iter_blocks(const char *dev, int (*callback)(void* buffer, uint64_t length)) {
	return FS_EXIT_OK;
}

int reiserfs_iter_blocks(const char *dev, int (*callback)(void* buffer, uint64_t length)) {
	return FS_EXIT_OK;
}

/************************************LOOKUP TABLE*******************************/
int fs_identify(const char *dev, int FS_TYPE) {
	switch(FS_TYPE) {
		case FS_EXT2_T:
			return ext2_identify(dev);
		
		case FS_XFS_T:
			return xfs_identify(dev);
		
		case FS_REISERFS_T:
			return reiserfs_identify(dev);
		
		default:
			return FS_EXIT_ER_ID_UNDEF;
	}
	return FS_EXIT_OK;
}

int fs_iter_blocks(const char *dev, int FS_TYPE, int (*callback)(void* buffer, uint64_t length)) {
	switch(FS_TYPE) {
		case FS_EXT2_T:
			return ext2_iter_blocks(dev, callback);
		
		case FS_XFS_T:
			return xfs_iter_blocks(dev, callback);
		
		case FS_REISERFS_T:
			return reiserfs_iter_blocks(dev, callback);
		
		default:
			return FS_EXIT_ER_ITER_UNDEF;
	}
	return FS_EXIT_OK;
}
