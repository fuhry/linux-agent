/**
	File:        reiser.c
	Author:      Charles Roydhouse
	Description: reiser filesystem parsing functions
*/

#include "reiser.h"
#include "fs.h"

int reiser_parse_superblock(int fd, struct reiser_fs *fs) {
	
	/** Seek to the signature */
	if(lseek(fd, REISER_SUPERBLOCK_LOC + REISER_SIGNATURE_OFF, SEEK_SET) < 0) {
		return FS_SEEK_ER;
	}

	if(read(fd, fs->signature, REISER_SIGNATURE_LEN) < 0) {
		return FS_READ_ER;
	}
	fs->signature[REISER_SIGNATURE_LEN - 1] = 0;
	
	return FS_EXIT_OK;
}
