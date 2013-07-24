/**
	File:        fsparser.h
	Author:      Charles Roydhouse
	Description: Provides a wrapper over POSIX/Linux for various filesystems
*/

#ifndef DATTO_FSPARSER_H
#define DATTO_FSPARSER_H

/** Exit status OK */
#define FS_EXIT_OK 0
/** Exit status unknown error */
#define FS_EXIT_ER_UNKOWN 1
/** Exit status undefined identifcation function */
#define FS_EXIT_ER_ID_UNDEF 2
/** Exit status undefined iteration function */
#define FS_EXIT_ER_ITER_UNDEF 3

#define FS_EXT2_T     1
#define FS_XFS_T      2
#define FS_REISERFS_T 3

#include <stdint.h>

/**
	Identifies if a given device is of a specific format.
	Params:  dev     - the device path
	         FS_TYPE - the type of filesystem to identify
	Returns: FALSE on success
	         TRUE on failure
*/
int fs_identify(const char *dev, int FS_TYPE);

/**
	Iterates over used blocks and callsback on said blocks.
	Params:  dev      - the device path
	         FS_TYPE  - the type of filesystem to expect
	         callback - the function to callback on a used block
	Returns: FALSE on success
	         TRUE on failure
*/
int fs_iter_blocks(const char *dev, int FS_TYPE, int (*callback)(void* buffer, uint64_t length));


#endif
