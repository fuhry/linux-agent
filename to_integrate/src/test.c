/**
	File:        test.c
	Author:      Charles Roydhouse
	Description: Tests functionality
*/

#include "fsparser.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define print_usage(exec) printf("Usage: %s [option]\n\nOptions:\n\t-i\tinput device (ex: /dev/sdb1)\n\t-o\toutput device (ex: /dev/loop1)\n\t-t\tfilesystem type (ex: ext, reiserfs, xfs, btrfs)\n", exec)

static int outfd = -1;
int test_callback(const int fd, const uint64_t length, uint64_t offset) {
	char buffer[length];
	read(fd, buffer, length);
	lseek(fd, offset, SEEK_SET); //seek back	
	lseek(outfd, offset, SEEK_SET); //seek to
	write(outfd, buffer, sizeof(char) * sizeof(buffer));
	return 0;
}

int main(int argc, char *argv[]) {	
	char *indev = NULL;
	int fstype = -1;
	
	int c;
	while((c = getopt (argc, argv, "i:o:t:")) != -1) {
		switch(c) {
			case 'i':
				indev = optarg;
			break;
		
			case 'o':
				if((outfd = open(optarg, O_WRONLY)) < 0) {
					fprintf(stderr, "Fatal: Could not open output device %s\n", optarg);
					return 1;
				}
			break;
		
			case 't':
				if(strncmp(optarg, "ext", 3) == 0)
					fstype = FS_EXT_T;
				else if(strncmp(optarg, "xfs", 3) == 0)
					fstype = FS_XFS_T;
				else if(strncmp(optarg, "btrfs", 4) == 0)
					fstype = FS_BTRFS_T;
				else if(strncmp(optarg, "reiserfs", 8) == 0)
					fstype = FS_REISERFS_T;
				else {
					fprintf(stderr, "Fatal: Invalid filesystem type %s\n", optarg);
					return 1;
				}
			break;
		
			case '?':
				print_usage(argv[0]);
			return 1;
			
			default:
			return 1;
		}
	}
	
	if(outfd < 0 || indev == NULL || fstype == -1) {
		print_usage(argv[0]);
		return 1;
	}
	
	if(fs_identify(indev, fstype)) {
		int bytes = fs_iter_blocks(indev, fstype, &test_callback);
		printf("%d\n", bytes);
	} else {
		fprintf(stderr, "Fatal: %s is not of supplied type!\n", indev);
		return 1;
	}
		
	if(outfd)
		close(outfd);
	return 0;
}
