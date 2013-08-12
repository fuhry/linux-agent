/**
	File:        test.c
	Author:      Charles Roydhouse
	Description: Tests functionality
*/

#include <stdio.h>
#include "fsparser.h"
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

#if !(defined TRUE && defined FALSE)
	#define TRUE 1
	#define FALSE 0
#endif

#define _TO_STR(n) #n
#define _BOOL_TO_STR(b) (b == 0 ? "FAIL" : "PASS")
#define _TEST_BASE(desc, func, value, comp) printf("[%s] Testing %s\n", _BOOL_TO_STR(func comp value), desc);
#define TEST_EQ(desc, func, value) _TEST_BASE(desc, func, value, ==)
#define TEST_NE(desc, func, value) _TEST_BASE(desc, func, value, !=)
#define TEST_GT(desc, func, value) _TEST_BASE(desc, func, value, >)
#define TEST_GTE(desc, func, value) _TEST_BASE(desc, func, value, >=)
#define TEST_LT(desc, func, value) _TEST_BASE(desc, func, value, <)
#define TEST_LTE(desc, func, value) _TEST_BASE(desc, func, value, <=)

#define XFS_TEST_PARTITION "/dev/sdb1"
#define REISER_TEST_PARTITION "/dev/sdb2"
#define EXT2_TEST_PARTITION "/dev/sdb3"
#define BTRFS_TEST_PARTITION "/dev/sdb4"

#define XFS_BYTES 5079040
#define REISER_BYTES 33665024
#define EXT2_BYTES 17035264
#define BTRFS_BYTES -1

static int out;
int test_callback(const int fd, const uint64_t length, uint64_t offset) {
/*
	char buffer[length];
	read(fd, buffer, length);
	lseek(fd, offset, SEEK_SET); //seek back
	
	lseek(out, offset, SEEK_SET); //seek to
	write(out, buffer, sizeof(char) * sizeof(buffer));
	*/
	//printf("Setting bit %ld\n", offset);
	return 0;
}

void test_xfs(void) {
	TEST_EQ("XFS identifcation", fs_identify(XFS_TEST_PARTITION, FS_XFS_T), TRUE)
	TEST_EQ("XFS misidentifcation", fs_identify(REISER_TEST_PARTITION, FS_XFS_T), FALSE)
	TEST_EQ("XFS misidentifcation", fs_identify(EXT2_TEST_PARTITION, FS_XFS_T), FALSE)
	TEST_EQ("XFS copied blocks", fs_iter_blocks(XFS_TEST_PARTITION, FS_XFS_T, &test_callback), XFS_BYTES)
}

void test_reiserfs(void) {
	TEST_EQ("ReiserFS identifcation", fs_identify(REISER_TEST_PARTITION, FS_REISERFS_T), TRUE)
	TEST_EQ("ReiserFS misidentifcation", fs_identify(XFS_TEST_PARTITION, FS_REISERFS_T), FALSE)
	TEST_EQ("ReiserFS misidentifcation", fs_identify(EXT2_TEST_PARTITION, FS_REISERFS_T), FALSE)
	TEST_EQ("ReiserFS copied blocks", fs_iter_blocks(REISER_TEST_PARTITION, FS_REISERFS_T, &test_callback), REISER_BYTES)
}

void test_ext2(void) {
	TEST_EQ("ext2 identifcation", fs_identify(EXT2_TEST_PARTITION, FS_EXT2_T), TRUE)
	TEST_EQ("ext2 misidentifcation", fs_identify(REISER_TEST_PARTITION, FS_EXT2_T), FALSE)
	TEST_EQ("ext2 misidentifcation", fs_identify(XFS_TEST_PARTITION, FS_EXT2_T), FALSE)
	TEST_EQ("ext2 copied blocks", fs_iter_blocks(EXT2_TEST_PARTITION, FS_EXT2_T, &test_callback), EXT2_BYTES)
}


void test_btrfs(void) {
	TEST_EQ("btrfs identifcation", fs_identify(BTRFS_TEST_PARTITION, FS_BTRFS_T), TRUE)
	TEST_EQ("btrfs misidentifcation", fs_identify(REISER_TEST_PARTITION, FS_EXT2_T), FALSE)
	TEST_EQ("btrfs misidentifcation", fs_identify(XFS_TEST_PARTITION, FS_EXT2_T), FALSE)
	TEST_EQ("btrfs copied blocks", fs_iter_blocks(BTRFS_TEST_PARTITION, FS_BTRFS_T, &test_callback), BTRFS_BYTES)
}


int main(int argc, char *argv[]) {
/*
	printf("Expecting XFS size ");printb(XFS_BYTES);
	printf("Expecting REISER size ");printb(REISER_BYTES);
	printf("Expecting EXT2 size ");printb(EXT2_BYTES);
*/
	if(argc != 3) {printf("WRONG ARGUMENTS!\n");return 1;}
	
	out = open(argv[1], O_WRONLY);
	switch(argv[2][0]) {
		case '1':
			test_xfs();
			break;
		case '2':
			test_reiserfs();
			break;
		case '3':
			test_ext2();
			break;
		case '4':
			test_btrfs();
			break;
		default:
			printf("WRONG FS ID\n");
			break;
	}
	close(out);
	return 0;
}
