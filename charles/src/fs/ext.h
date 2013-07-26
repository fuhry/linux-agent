/**
	File:        ext.h
	Author:      Charles Roydhouse
	Description: ext filesystem parsing functions
*/

#ifndef DATTO_EXT_H
#define DATTO_EXT_H

#include <stdint.h>

/** EXT's signature */
#define EXT_SIGNATURE 0x53ef

/**
	Parses the signature for the filesystem and compares it to the expected result.
	Params:  fd - the file descriptor
	Returns: TRUE on success
	         FALSE on failure
*/
int ext_has_identifier(int fd);

/**
	Iterates over used blocks and callsback on said blocks.
	Params:  dev      - the device
	         callback - the function to callback on a used block
	Returns: FALSE on success
	         TRUE on failure
*/
int ext_iter_blocks(const char *dev, int (*callback)(int fd, uint64_t length));

#endif
