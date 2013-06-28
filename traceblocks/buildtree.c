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
	struct blk_io_trace buf;
	long total_read_bytes;

	unsigned int i;
	
	range_tree = malloc(sizeof(struct rb_root));
	if (range_tree == NULL) {
		perror("malloc");
		return 1;
	}

	printf("sizeof: %d\n", sizeof(struct blk_io_trace));
	while ((read_bytes =
				fread(&buf, sizeof(struct blk_io_trace), 1, stdin) > 0)) {
		total_read_bytes += read_bytes;
		if (buf.magic != (BLK_IO_TRACE_MAGIC | BLK_IO_TRACE_VERSION)) {
			fprintf(stderr, "got magic: 0x%x\n", buf.magic);
			fprintf(stderr, "total_bytes: %ld\n", total_read_bytes);
			break;
		}
		for (i = 0; i < buf.bytes / 512; i++) {
			add_value(buf.sector + i, range_tree);
		}
		/* Skip over the extra data */
		fread(&buf, 1, buf.pdu_len, stdin);
	}
	if (read_bytes != 0) {
		perror("read");
	}

	/* Iterate through the tree and free each node */
	while ((node = rb_first(range_tree))) {
		range = rb_entry(node, struct range, range_node);
		printf("%ld-%ld\n", range->start, range->end);
		rb_erase(node, range_tree);
		free(range);
	}
	return 0;
}
