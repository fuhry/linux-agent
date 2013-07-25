/**
	File:        xfs.h
	Author:      Charles Roydhouse
	Description: xfs filesystem parsing functions
*/

#ifndef DATTO_XFS_H
#define DATTO_XFS_H

#include <stdint.h>

/** XFS's signature */
#define XFS_SIGNATURE 0x58465342

/** Superblock location */
#define XFS_SUPERBLOCK_LOC 0x00

/** Signature field offset from the superblock */
#define XFS_SIGNATURE_OFF 0x00

/** xfs filesystem information structure */
struct xfs_fs {
	uint32_t signature;
};

/**
	Parses necessary information into the filesystem structure
	Params:  fd - the file descriptor
	         fs - the xfs filesystem structure
	Returns: FALSE on success
	         TRUE on failure
*/
int xfs_parse_information(int fd, struct xfs_fs *fs);

#endif
