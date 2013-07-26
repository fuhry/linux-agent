/**
	File:        fsparser.c
	Author:      Charles Roydhouse
	Description: Provides implementation for a wrapper over POSIX/Linux for various filesystems
	TODO: convert XFS/REISER to the model that EXT is developing into
*/

#include "fsparser.h"
#include "fs/ext.h"
#include "fs/xfs.h"
#include "fs/reiser.h"
#include <fcntl.h>
#include <unistd.h>

#if !(defined TRUE && defined FALSE)
	#define TRUE 1
	#define FALSE 0
#endif

int fs_identify(const char *dev, const int FS_TYPE) {
	int fd = open(dev, O_RDONLY);
	int rc = 0;
	if(fd < 0) { 
		return FALSE;
	}	
	switch(FS_TYPE) {
		case FS_EXT2_T:
			rc = ext_has_identifier(fd);
			break;
		
		case FS_XFS_T:
			rc =  xfs_has_identifier(fd);
			break;
					
		case FS_REISERFS_T:
			rc = reiser_has_identifier(fd);
			break;
			
		default:
			close(fd);
			return FALSE;
	}	
	if(close(fd)) {
			return FALSE;
	}
	return rc;
}



int fs_iter_blocks(const char *dev, const int FS_TYPE, int (*callback)(int fd, uint64_t length)) {
	int rc;
	switch(FS_TYPE) {
		case FS_EXT2_T:
			rc = ext_iter_blocks(dev, callback);
			break;
		
		case FS_XFS_T:
			rc = xfs_iter_blocks(dev, callback);
			break;
		
		case FS_REISERFS_T:
			rc = reiser_iter_blocks(dev, callback);
			break;
		
		default:
			return FS_EXIT_ER_ITER_UNDEF;
	}
	return rc;
}
