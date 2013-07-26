/**
	File:        reiser.c
	Author:      Charles Roydhouse
	Description: reiser filesystem parsing functions
*/

#include "reiser.h"
#include "fs.h"
#include <string.h>
#include <stdbool.h>

int reiser_has_identifier(int fd) {
	char signature[REISER_SIGNATURE_LEN];
	
	/** Seek to the signature */
	if(lseek(fd, REISER_SUPERBLOCK_LOC + REISER_SIGNATURE_OFF, SEEK_SET) < 0) {
		return false;
	}

	if(read(fd, signature, REISER_SIGNATURE_LEN) < 0) {
		return false;
	}
	signature[REISER_SIGNATURE_LEN - 1] = 0;	
	return strncmp(signature, REISER_SIGNATURE, REISER_SIGNATURE_LEN) == 0;
}



int reiser_iter_blocks(const char *dev, int (*callback)(int fd, uint64_t length)) {
	return true;
}
