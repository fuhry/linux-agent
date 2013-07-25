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

/** Superblock location */
#define EXT_SUPERBLOCK_LOC 0x400

/** Block count field offset from superblock */
#define EXT_BLOCKCOUNT_OFF 0x004

/** Block size shift amount field offset from superblock */
#define EXT_BLOCKSHIFT_OFF 0x018

/** Signature field offset from superblock */
#define EXT_SIGNATURE_OFF 0x038

/** Blocks per group field offset from superblock */
#define EXT_BLOCKSPERGROUP_OFF 0x01c


/** ext filesystem information structure */
struct ext_fs {
	uint16_t signature;
	uint32_t total_blocks;
	uint32_t total_groups;
	uint64_t block_size;
	uint32_t blocks_per_group;
	uint32_t first_group_desc;
};

/**
	Parses necessary information into the filesystem structure
	Params:  fd - the file descriptor
	         fs - the ext filesystem structure
	Returns: FALSE on success
	         TRUE on failure
*/
int ext_parse_information(int fd, struct ext_fs *fs);

#endif
