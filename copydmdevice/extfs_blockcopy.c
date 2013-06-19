#include <errno.h>
#include <error.h>
#include <ext2fs/ext2_err.h>
#include <ext2fs/ext2_io.h>
#include <ext2fs/ext2fs.h>
#include <stdio.h>
#include <stdlib.h>

int extfs_copy(const char *, const char *, unsigned long *);

static inline int round_up(int num, unsigned int mult)
{
	return mult * (((num - 1) / mult) + 1);
}

int extfs_copy(const char *source, const char *dest,
		unsigned long *blocks_copied)
{
	int r = 0;

	int extfs_err = 0;

	unsigned int i;
	unsigned int j;

	ext2_filsys fs = NULL;
	int b_size_bytes;

	FILE *source_fd = NULL;
	FILE *dest_fd = NULL;
	void *block_bitmap = NULL;
	void *block_buf = NULL;

	off_t cur_group_block_offset;
	off_t cur_block_offset;
	off_t seek_amt;

	add_error_table(&et_ext2_error_table);

	if ((source_fd = fopen(source, "r")) == NULL) {
		error(0, errno, "Error opening %s", source);
		goto out;
	}

	if ((dest_fd = fopen(dest, "w")) == NULL) {
		error(0, errno, "Error opening %s", dest);
		goto out;
	}

	if ((extfs_err = ext2fs_open(source, 0, 0, 0, unix_io_manager, &fs))) {
		error(0, errno, "Unable to open %s as extfs - %s", source,
				error_message(extfs_err));
		goto out;
	}

	if ((extfs_err = ext2fs_read_bitmaps(fs))) {
		error(0, errno, "Unable to read %s bitmap - %s", source,
				error_message(extfs_err));
		goto out;
	}

	b_size_bytes = 1024 << fs->super->s_log_block_size;
	block_buf = malloc(b_size_bytes);
	if (block_buf == NULL) {
		perror("malloc");
		goto out;
	}

	/* one bit per block */
	block_bitmap = malloc(round_up(fs->super->s_blocks_per_group, 8) / 8);
	if (block_bitmap == NULL) {
		perror("malloc");
		goto out;
	}

	if (blocks_copied) {
		*blocks_copied = 0;
	}

	for (i = 0; i < fs->group_desc_count; i++) {
		cur_group_block_offset = fs->super->s_first_data_block +
			(i * fs->super->s_blocks_per_group);

		ext2fs_get_block_bitmap_range(fs->block_map, cur_group_block_offset,
				fs->super->s_blocks_per_group,
				block_bitmap);

		/* Break if we have reached the end of this group (s_blocks_per_group)
		 * or the end of the file system (s_blocks_count) */
		for (j = 0; j < fs->super->s_blocks_per_group &&
				j + cur_group_block_offset < fs->super->s_blocks_count; j++) {

			if (ext2fs_test_bit(j, block_bitmap) ||
					j < fs->super->s_first_data_block) {
				cur_block_offset = cur_group_block_offset + j;
				seek_amt = b_size_bytes * cur_block_offset;

				if (fseeko(source_fd, seek_amt, SEEK_SET) == (off_t)(-1)) {
					error(0, errno, "Error seeking %s (Block: %ld)",
							source, (long)cur_block_offset);
					goto out;
				}
				if (fseeko(dest_fd, seek_amt, SEEK_SET) == (off_t)(-1)) {
					error(0, errno, "Error seeking %s (Block: %ld)", dest,
							(long)cur_block_offset);
					goto out;
				}

				if (fread(block_buf, b_size_bytes, 1, source_fd) != 1) {
					error(0, errno, "Error reading %s", source);
					goto out;
				}
				if (fwrite(block_buf, b_size_bytes, 1, dest_fd) != 1) {
					error(0, errno, "Error writing %s", dest);
					goto out;
				}

				if (blocks_copied) {
					(*blocks_copied)++;
				}
			}
		}
	}

	r = 1;

out:
	if (source_fd != NULL) {
		fclose(source_fd);
	}

	if (dest_fd != NULL) {
		fclose(dest_fd);
	}

	if (fs != NULL) {
		ext2fs_close(fs);
	}

	free(block_bitmap);

	free(block_buf);

	remove_error_table(&et_ext2_error_table);

	return r;
}
