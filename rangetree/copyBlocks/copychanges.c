#include "rangetree.h"
#include <stdio.h>
#include <errno.h>

#define BYTES_PER_SEC 512

/* copy sectors from src_fd to dest_fd within the parameters of sector_range*/
int copy_sector_range(int src_fd, int dest_fd, struct range *sector_range)
{
    int i;
    off_t offset = (sector_range->start)*BYTES_PER_SEC;
    char buf[BYTES_PER_SEC];
    ssize_t io_bytes = 0;

    /* update the file descriptors with the initial offset */
    lseek(src_fd, offset, SEEK_SET);
    lseek(dest_fd, offset, SEEK_SET);
    
    for (i = sector_range->start; i < sector_range->end; i++) {
        /* read data from src_fd into a temporary buffer */
        io_bytes = read(src_fd, buf, BYTES_PER_SEC);
        if (io_bytes != BYTES_PER_SEC)
            return io_bytes;

        /* write data from the buffer to dest_fd */
        io_bytes = write(dest_fd, buf, BYTES_PER_SEC);
        if (io_bytes != BYTES_PER_SEC)
            return io_bytes;
        
        /* increment the offset*/
        offset += BYTES_PER_SEC;
    }

    return 0;
}

/* traverse the range tree and copy each range from src_fd to dest_fd */
int copy_changes(int src_fd, int dest_fd, struct rb_root *root)
{
    struct rb_node *node = NULL;
    struct range *range = NULL;
    
    node = rb_first(root);
    if (node) {
        do {
            range = rb_entry(node, struct range, range_node);
            if(copy_sector_range(src_fd, dest_fd, range))
                return errno;
        } while ((node = rb_next(node)));
    }

    return 0;
}
