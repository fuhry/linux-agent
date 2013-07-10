#include "rangetree.h"
#include <linux/blktrace_api.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
	struct rb_root *range_tree = NULL;
	struct rb_node *node = NULL;
	struct range *range = NULL;

	int read_bytes = -1;
	struct blk_io_trace trace;
	long total_read_bytes;
	unsigned int num_sectors;

	unsigned int i;
	
	range_tree = malloc(sizeof(struct rb_root));
	if (range_tree == NULL) {
		perror("malloc");
		return 1;
	}

	/* From stdin, read in a blk_io_trace struct. stdin should be the output
	 * from blktrace or something similar */
	while ((read_bytes = fread(&trace, sizeof(struct blk_io_trace), 1,
					stdin) > 0)) {
		total_read_bytes += read_bytes;

		/* Sanity check, make sure the blk_io_trace struct has the right format */
		if (trace.magic != (BLK_IO_TRACE_MAGIC | BLK_IO_TRACE_VERSION)) {
			fprintf(stderr, "got magic: 0x%x\n", trace.magic);
			fprintf(stderr, "total_bytes: %ld\n",
					total_read_bytes);
			break;
		}

		
		/* Calculate the number of sectors in this trace */
		num_sectors = trace.bytes / 512;

		/* Add each sector to the tree */
		for (i = 0; i < num_sectors; i++) {
			rt_add_value(trace.sector + i, range_tree);
		}

		/* Skip over the extra data at end of trace buffer */
		fread(&trace, 1, trace.pdu_len, stdin);
	}
	if (read_bytes != 0) {
		perror("read");
	}

	node = rb_first(range_tree);
	while ((node = rb_next(node))) {
		range = rb_entry(node, struct range, range_node);
		printf("%ld-%ld\n", range->start, range->end);
	}

	rt_free_tree(range_tree);

	return 0;
}
