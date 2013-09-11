/**
	File:        xfs.h
	Author:      Charles Roydhouse
	Description: xfs filesystem parsing functions
*/

#ifndef DATTO_XFS_H
#define DATTO_XFS_H

#include <stdint.h>

/**
	Parses the signature for the filesystem and compares it to the expected
	result.
	Params:  fd - the file descriptor
	Returns: TRUE on success
	         FALSE on failure
*/
int xfs_has_identifier(int fd);

/**
	Iterates over used blocks and callsback on said blocks.
	Params:  dev      - the device
	         callback - the function to callback on a used block
	Returns: total number of bytes read
*/
int xfs_iter_blocks(const char *dev,
	int (*callback)(int fd, uint64_t length, uint64_t offset));

#endif
