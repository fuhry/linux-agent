#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <blkid/blkid.h>

int main(int argc, char **argv) {
	char *fs = blkid_get_tag_value(NULL, "TYPE", argv[1]);
	printf("%s\n", fs);
	return 0;
}
