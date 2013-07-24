/**
	File:        test.c
	Author:      Charles Roydhouse
	Description: Tests functionality
*/

#include <stdio.h>
#include "fsparser.h"

#define _TO_STR(n) #n
#define _BOOL_TO_STR(b) (b == 0 ? "FALSE" : "TRUE")
#define TEST_SUCCESS(func, value) printf("Calling `%s` expecting `%s` .....\t", _TO_STR(func), _TO_STR(value)); printf("%s\n", _BOOL_TO_STR(func == value));
#define TEST_FAILURE(func, value) printf("Calling `%s` not expecting `%s` .....\t", _TO_STR(func), _TO_STR(value)); printf("%s\n", _BOOL_TO_STR(func == value));

#define XFS_TEST_PARTITION "/dev/sdb1"
#define REISER_TEST_PARTITION "/dev/sdb2"
#define EXT2_TEST_PARTITION "/dev/sdb3"

void test_xfs(void) {

}

void test_reiserfs(void) {

}

void test_ext2(void) {
	int rc = 0;
	rc = fs_identify(EXT2_TEST_PARTITION, FS_EXT2_T);
	TEST_SUCCESS(rc, 1)	
}

int main(void) {
	test_ext2();
	return 0;
}
