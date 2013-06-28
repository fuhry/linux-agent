#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>

/* While we could do detection on if a device is a DM device or not, we
 * force the user to explicitly declare the device is a regular block device.
 * Non-DM devices should only be backed up under unusual circumstances.
 */
enum {
	NON_DM = 0
};

int setup_cow_device(const char *, const char *, char *);
int takedown_cow_device(const char *);
int extfs_copy(const char *, const char *, unsigned long *);

int main(int argc, char **argv)
{
	const char *source_path;
	const char *dest_path;
	const char *mem_dev;
	const char *copy_from;

	char cow_path[PATH_MAX];

	unsigned long blocks_copied = 0;

	int c;
	int is_dm_dev = 1;
	const char *prog_name;

	prog_name = argv[0];

	static struct option long_options[] =
	{
		{"nondm",	no_argument,	0,	NON_DM}
	};

	while ((c = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
		switch (c) {
		case NON_DM:
			is_dm_dev = 0;
			break;
		case '?':
			break;
		default:
			fprintf(stderr, "Reached unexpected branch\n");
		}
	}
	/* argc is now the number of non-option arguments and does not include the
	 * program name */
	argc -= optind;
	argv += optind;

	if ((is_dm_dev && argc != 3) || (!is_dm_dev && argc != 2)) {
		fprintf(stderr,
				"usage: %s [--nondm] dm_block_dev dest_path [mem_dev]\n",
				prog_name);
		return 1;
	}

	source_path = argv[0];
	dest_path = argv[1];
	if (is_dm_dev)
		mem_dev = argv[2];

	if (is_dm_dev) {
		if (!setup_cow_device(source_path, mem_dev, cow_path)) {
			error(0, errno, "Error setting up COW device for %s", argv[1]);
			return 1;
		}
		copy_from = cow_path;
	} else {
		copy_from = source_path;
	}

	if (!extfs_copy(copy_from, dest_path, &blocks_copied)) {
		error(0, errno, "Error copying blocks from COW device");
		/* Don't return on error as we still need to clean up */
	}

	printf("Copied %ld blocks to %s\n", blocks_copied, dest_path);

	if (is_dm_dev) {
		if (!takedown_cow_device(source_path)) {
			error(0, errno, "Error taking down COW device for %s", argv[1]);
			return 1;
		}
	}

	return 0;
}
