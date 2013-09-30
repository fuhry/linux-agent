/**
	File:        fsparser.h
	Author:      Charles Roydhouse
	Description: Provides a wrapper over POSIX/Linux for various filesystems
*/

#ifndef DATTO_FSPARSER_H
#define DATTO_FSPARSER_H

/** Exit status undefined iteration function */
#define FS_EXIT_ER_ITER_UNDEF -1

/** EXT2 family filesystem type */
#define FS_EXT_T     1

/** XFS filesystem type */
#define FS_XFS_T      2

/** REISERFS filesystem type */
#define FS_REISERFS_T 3

/** BTRFS filesystem type */
#define FS_BTRFS_T 4

#include <stdint.h>

/**
	Identifies if a given device is of a specific format.
	Params:  dev     - the device path
	         FS_TYPE - the type of filesystem to identify
	Returns: TRUE on success
	         FALSE on failure
*/
int fs_identify(const char *dev, const int FS_TYPE);

/**
	Iterates over used blocks and callsback on said blocks.
	Params:  dev      - the device
	         FS_TYPE  - the type of filesystem to expect
	         callback - the function to callback on a used block
	Returns: total number of bytes read
*/
int fs_iter_blocks(const char *dev, const int FS_TYPE,
	int (*callback)(int fd, uint64_t length, uint64_t offset));


#endif
