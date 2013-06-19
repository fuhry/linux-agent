#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <error.h>
#include <errno.h>

int setup_cow_device(const char *, const char *, char *);
int takedown_cow_device(const char *);
int extfs_copy(const char *, const char *, unsigned long *);

int main(int argc, char **argv)
{
	const char *dm_device_path;
	const char *dest_path;
	const char *mem_dev;

	char cow_path[PATH_MAX];

	unsigned long blocks_copied = 0;

	if (argc != 4) {
		fprintf(stderr, "usage: %s dm_block_dev dest_path mem_dev\n",
				argv[0]);
		return 1;
	}

	dm_device_path = argv[1];
	dest_path = argv[2];
	mem_dev = argv[3];

	if (!setup_cow_device(dm_device_path, mem_dev, cow_path)) {
		error(0, errno, "Error setting up COW device for %s", argv[1]);
		return 1;
	}

	if (!extfs_copy(cow_path, dest_path, &blocks_copied)) {
		error(0, errno, "Error copying blocks from COW device");
		/* Don't return on error as we still need to clean up */
	}

	printf("Copied %ld blocks to %s\n", blocks_copied, dest_path);

	if (!takedown_cow_device(dm_device_path)) {
		error(0, errno, "Error taking down COW device for %s", argv[1]);
		return 1;
	}

	return 0;
}
