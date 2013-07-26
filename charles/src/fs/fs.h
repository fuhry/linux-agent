/**
	File:        fs.h
	Author:      Charles Roydhouse
	Description: Useful macros for filesystem reads
*/

#ifndef DATTO_FS_H
#define DATTO_FS_H

#include <fcntl.h>
#include <unistd.h>

#define letobe16(x) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))
#define letobe32(x) (((x) & 0xff) << 24) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | (((x) >> 24) & 0xff)

#if !(defined TRUE && defined FALSE)
	#define TRUE 1
	#define FALSE 0
#endif

#define FS_EXIT_OK 0
#define FS_SEEK_ER 1
#define FS_READ_ER 2

#endif
