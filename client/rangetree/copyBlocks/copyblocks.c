#include "../rangetree.h"
#include <linux/blktrace_api.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define BYTES_PER_SEC 512

int copy_changes(int src_fd, int dest_fd, struct rb_root *root);

int main(int argc, char* argv[])
{
	struct rb_root *range_tree = NULL;
	struct blk_io_trace trace;
	long total_read_bytes = 0;
	int read_bytes = -1;
    int src_fd = -1;
    int dest_fd = -1;
	unsigned int num_sectors;
	unsigned int i;
	
	range_tree = malloc(sizeof(struct rb_root));
	if (range_tree == NULL) {
		perror("malloc");
		return 1;
	}
    
    if (argc != 3) {
        printf("usage: %s file1 file2\n", argv[0]); 
        return 1;
    }
    
    /* open src_fd and dest_fd */
    src_fd = open(argv[1], O_RDONLY);
    if (src_fd == -1) {
        printf("Error opening %s: %s\n", argv[1], strerror(errno));
        return 1;
    }
    
    dest_fd = open(argv[2], O_WRONLY);
    if (dest_fd == -1) {
        printf("Error opening %s: %s\n", argv[2], strerror(errno));
        return 1;
    }
    
	/* From stdin, read in a blk_io_trace struct. stdin should be the output
	 * from blktrace or something similar */
	while ((read_bytes = fread(&trace, sizeof(struct blk_io_trace), 1,
					stdin) > 0)) {
		total_read_bytes += read_bytes;

		/* Sanity check, make sure the blk_io_trace struct has the right format */
		if (trace.magic != (BLK_IO_TRACE_MAGIC | BLK_IO_TRACE_VERSION)) {
			fprintf(stderr, "Error checking magic number\n");
			fprintf(stderr, "  got magic: 0x%x\n", trace.magic);
			fprintf(stderr, "  total_bytes: %ld\n",
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
    
	if (total_read_bytes <= 0) {
		perror("read");
    }
    
    if(copy_changes(src_fd, dest_fd, range_tree)) {
        printf("Error copying changes\n");
    }
    
	rt_free_tree(range_tree);
    
	return 0;
}
