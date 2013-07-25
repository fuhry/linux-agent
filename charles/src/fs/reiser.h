/**
	File:        reiser.h
	Author:      Charles Roydhouse
	Description: reiser filesystem parsing functions
*/

#ifndef DATTO_REISER_H
#define DATTO_REISER_H

/** REISER's signature */
#define REISER_SIGNATURE "ReIsEr2Fs"
#define REISER_SIGNATURE_LEN 12

/** Superblock location */
#define REISER_SUPERBLOCK_LOC 0x10000

/** Signature field offset from superblock */
#define REISER_SIGNATURE_OFF 0x034

/** reiser filesystem information structure */
struct reiser_fs {
	char signature[REISER_SIGNATURE_LEN];
};

/**
	Parses necessary information into the filesystem structure
	Params:  fd - the file descriptor
	         fs - the xfs filesystem structure
	Returns: FALSE on success
	         TRUE on failure
*/
int reiser_parse_superblock(int fd, struct reiser_fs *fs);


#endif
