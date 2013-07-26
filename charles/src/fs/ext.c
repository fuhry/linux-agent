/**
	File:        ext.c
	Author:      Charles Roydhouse
	Description: ext filesystem parsing functions
*/

#include "ext.h"
#include "fs.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <errno.h>
#include <error.h>
#include <ext2fs/ext2_err.h>
#include <ext2fs/ext2_io.h>
#include <ext2fs/ext2fs.h>

#define EXT_SUPERBLOCK_LOC 0x400
#define EXT_SUPERBLOCK_SIZ 0x400

int ext_has_identifier(int fd) {
	uint16_t signature;
	struct ext2_super_block super;

	/* Seek to superblock */
	if(lseek(fd, EXT_SUPERBLOCK_LOC, SEEK_SET) < 0) {
		return FS_SEEK_ER;
	}
	
	/* Read superblock into struct */
	if(read(fd, &super, EXT_SUPERBLOCK_SIZ) < 0) {
		return FS_READ_ER;
	}
	
	/* Switch byte-order to big endian if needed */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	signature = letobe16(super.s_magic);
#endif
	return signature == EXT_SIGNATURE;
}



int ext_iter_blocks(const char *dev, int (*callback)(int fd, uint64_t length)) {
	int rc = FS_EXIT_OK;
	ext2_filsys fs = NULL;
	
	add_error_table(&et_ext2_error_table);
	if((rc = ext2fs_open(dev, 0, 0, 0, unix_io_manager, &fs))) {
		error(0, errno, "Unable to open %s as extfs - %s", dev, error_message(rc));
		goto out;
	}
	
out:
	if (fs != NULL) {
		ext2fs_close(fs);
	}
	return rc;
}
