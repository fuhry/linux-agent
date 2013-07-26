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

/** Exit status open failure */
#define FS_EXIT_ER_OPEN 4

/** Exit status close failure */
#define FS_EXIT_ER_CLOSE 5

/** Exit status parse failure */
#define FS_EXIT_ER_PARSE 6


/** EXT2 family filesystem type */
#define FS_EXT2_T     1

/** XFS filesystem type */
#define FS_XFS_T      2

/** REISERFS filesystem type */
#define FS_REISERFS_T 3

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
	Returns: FALSE on success
	         TRUE on failure (see FS_EXIT macros for corresponding error)
*/
int fs_iter_blocks(const char *dev, const int FS_TYPE, int (*callback)(int fd, uint64_t length));


#endif
