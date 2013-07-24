/**
	File:        fsparser.c
	Author:      Charles Roydhouse
	Description: Provides implementation for a wrapper over POSIX/Linux for various filesystems
*/

#include "fsparser.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define letobe16(x) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))
#define letobe32(x) (((x) & 0xff) << 24) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | (((x) >> 24) & 0xff)
		
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
	int rc = 0;
	union {
		uint16_t u16;
		uint8_t byte[sizeof(uint16_t)];
	} signature;
	
	if(fd < 0) { 
		fprintf(stderr, "Could not open device `%s`\n", dev);
		return FALSE;
	}
	
	if(lseek(fd, 0x438, SEEK_CUR) < 0) {
		fprintf(stderr, "Could not seek device `%s`\n", dev);
		close(fd);
		return FALSE;
	}

	if(read(fd, signature.byte, sizeof(signature)) < 0) {
		fprintf(stderr, "Could not read device `%s`\n", dev);
		close(fd);
		return FALSE;
	}
	
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	signature.u16 = letobe16(signature.u16);
#endif
	
	rc = close(fd);
	rc = (signature.u16 == 0x53ef) && (rc == 0);
	return rc;
}

int xfs_identify(const char *dev) {
	int fd = open(dev, O_RDONLY);
	int rc = 0;
	union {
		uint32_t u32;
		uint8_t byte[sizeof(uint32_t)];
	} signature;
	
	if(fd < 0) { 
		fprintf(stderr, "Could not open device `%s`\n", dev);
		return FALSE;
	}
	
	if(lseek(fd, 0x0, SEEK_CUR) < 0) {
		fprintf(stderr, "Could not seek device `%s`\n", dev);
		close(fd);
		return FALSE;
	}

	if(read(fd, signature.byte, sizeof(signature)) < 0) {
		fprintf(stderr, "Could not read device `%s`\n", dev);
		close(fd);
		return FALSE;
	}
	
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	signature.u32 = letobe32(signature.u32);
#endif

	rc = close(fd);
	rc = (signature.u32 == 0x58465342) && (rc == 0);
	return rc;
}

int reiserfs_identify(const char *dev) {
	int fd = open(dev, O_RDONLY);
	int rc = 0;
	char* expected_signature = "ReIsEr2Fs";
	int expected_length = strlen(expected_signature);
	char signature[12];
	
	if(fd < 0) { 
		fprintf(stderr, "Could not open device `%s`\n", dev);
		return FALSE;
	}
	
	if(lseek(fd, 0x10034, SEEK_CUR) < 0) {
		fprintf(stderr, "Could not seek device `%s`\n", dev);
		close(fd);
		return FALSE;
	}
	
	if(read(fd, signature, 12) < 0) {
		fprintf(stderr, "Could not read device `%s`\n", dev);
		close(fd);
		return FALSE;
	}
	signature[expected_length] = 0;
	rc = close(fd);
	rc = (strncmp(signature, expected_signature, expected_length) == 0) && (rc == 0);
	return rc;
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
