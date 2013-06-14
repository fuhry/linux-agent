#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ext2fs/ext2_err.h>
#include <ext2fs/ext2fs.h>
#include <ext2fs/ext2_io.h>

int extfs_copy(const char *source, const char *dest);
int blkcp_err = 0;

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("usage: %s ext_block_dev dest_block_dev\n", argv[0]);
        return EINVAL;
    }

    char *source = argv[1];
    char *dest = argv[2];

    int blks_copied = extfs_copy(source, dest);

    if (blkcp_err) {
        fprintf(stderr, "Error code: %d\n", blkcp_err);
        return blkcp_err;
    }
    else {
        printf("Copied %d blocks.\n", blks_copied);
    }

    return 0;
}

static inline int round_up(int num, unsigned int mult) {
    return mult * (((num - 1) / mult) + 1);
}

int extfs_copy(const char *source, const char *dest) {
    int blocks_copied = 0;
    int block_size_bytes;
    int i;
    int j;
    ext2_filsys fs;
    FILE *source_fd = NULL;
    FILE *dest_fd = NULL;
    void *block_bitmap = NULL;
    void *block_buf = NULL;
    off_t cur_group_block_offset;
    off_t cur_block_offset;

    initialize_ext2_error_table();

    if ((source_fd = fopen(source, "r")) == NULL) {
        blkcp_err = errno;
        printf("Error opening %s: %s\n", source, strerror(blkcp_err));
        goto out;
    }

    if ((dest_fd = fopen(dest, "w")) == NULL) {
        blkcp_err = errno;
        printf("Error opening %s: %s\n", dest, strerror(blkcp_err));
        goto out;
    }

    if (blkcp_err = ext2fs_open(source, 0, 0, 0, unix_io_manager, &fs)) {
        fprintf(stderr, "Unable to open %s as extfs, 0x%08x %s\n", source, blkcp_err, error_message(blkcp_err));
        goto out;
    }

    if (blkcp_err = ext2fs_read_bitmaps(fs)) {
        fprintf(stderr, "Unable to read bitmap of %s, 0x%x %s\n", source, blkcp_err, error_message(blkcp_err));
        goto out;
    }

    block_size_bytes = 1024 << fs->super->s_log_block_size;
    block_buf = malloc(block_size_bytes);
    if (block_buf == NULL) {
        blkcp_err = errno;
        fprintf(stderr, "Error allocating memory: %s", strerror(blkcp_err));
        goto out;
    }

    // one bit per block
    block_bitmap = malloc(round_up(fs->super->s_blocks_per_group, 8) / 8);
    if (block_bitmap == NULL) {
        blkcp_err = errno;
        fprintf(stderr, "Error allocating memory: %s", strerror(blkcp_err));
        goto out;
    }

    for (i = 0; i < fs->group_desc_count; i++) {
        cur_group_block_offset = i * fs->super->s_blocks_per_group;
        ext2fs_get_block_bitmap_range(fs->block_map, cur_group_block_offset,
                                      fs->super->s_blocks_per_group, block_bitmap);
        for (j = 0; j < fs->super->s_blocks_per_group && j + cur_group_block_offset < fs->super->s_blocks_count; j++) {
            if (ext2fs_test_bit(j, block_bitmap)) {
                cur_block_offset = cur_group_block_offset + j;
                if (fseeko(source_fd, block_size_bytes * cur_block_offset, SEEK_SET) == (off_t)(-1)) {
                    blkcp_err = errno;
                    printf("Error seeking %s: %s. (Block: %d)\n", source, strerror(blkcp_err), (int)cur_block_offset);
                    goto out;
                }
                if (fseeko(dest_fd, block_size_bytes * cur_block_offset, SEEK_SET) == (off_t)(-1)) {
                    blkcp_err = errno;
                    printf("Error seeking %s: %s. (Block: %d)\n", dest, strerror(blkcp_err), (int)cur_block_offset);
                    goto out;
                }
                if (fread(block_buf, block_size_bytes, 1, source_fd) != 1) {
                    blkcp_err = errno;
                    printf("Error reading %s: %s\n", source, strerror(blkcp_err));
                    goto out;
                }

                if (fwrite(block_buf, block_size_bytes, 1, dest_fd) != 1) {
                    blkcp_err = errno;
                    printf("Error writing %s: %s\n", dest, strerror(blkcp_err));
                    goto out;
                }
                blocks_copied++;
            }
        }
    }

out:
    if (source_fd != NULL) {
        fclose(source_fd);
    }

    if (dest_fd != NULL) {
        fclose(dest_fd);
    }

    if (block_bitmap != NULL) {
        free(block_bitmap);
    }

    if (block_buf != NULL) {
        free(block_buf);
    }

    ext2fs_close(fs);

    if (blkcp_err) {
        return -1;
    }
    return blocks_copied;
}
