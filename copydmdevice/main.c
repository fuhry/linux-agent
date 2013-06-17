#include <stdlib.h>
#include <stdio.h>

int setup_cow_device(char *dm_block_dev, char *mem_dev);

int main(int argc, char **argv) {
	char *dm_device_path;
	// char *dest_block_dev;
	char *mem_dev;

	if (argc != 4) {
		fprintf(stderr, "usage: %s dm_block_dev dest_block_dev mem_dev\n",
				argv[0]);
		return 1;
	}

	dm_device_path = argv[1];
	// dest_block_dev = argv[2];
	mem_dev = argv[3];

	if (!setup_cow_device(dm_device_path, mem_dev)) {
		fprintf(stderr, "Error setting up COW device for %s\n", argv[1]);
		return 1;
	}

	return 0;
}
