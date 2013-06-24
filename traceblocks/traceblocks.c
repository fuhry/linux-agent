#include <error.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/blktrace_api.h>
#include <linux/fs.h>
#include <linux/limits.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#define DEBUG_FS_PATH "/sys/kernel/debug"
#define READ_BUF_SIZE 1024

int main(int argc, char **argv) {
	int dev_fd = -1;
	int trace_fd;
	int num_cpus;
	void *buf;
	size_t read_bytes;
	size_t total_read_bytes = 0;
	char trace_path[PATH_MAX];
	struct blk_user_trace_setup buts;
	struct pollfd *pfds;
	int i;

	if (argc != 2) {
		fprintf(stderr, "usage: %s /dev/<block_dev>\n", argv[0]);
		return 1;
	}

	if ((dev_fd = open(argv[1], O_RDONLY | O_NONBLOCK)) == -1) {
		perror("open");
		return 1;
	}

	memset(&buts, 0, sizeof(buts));
	buts.buf_size = 1024 * 1024;
	buts.buf_nr = 4;
	buts.act_mask = BLK_TC_WRITE | BLK_TC_QUEUE;

	if (ioctl(dev_fd, BLKTRACESETUP, &buts))
		perror("ioctl: BLKTRACESETUP");

	if (ioctl(dev_fd, BLKTRACESTART))
		perror("ioctl: BLKTRACESTART");

    num_cpus = get_nprocs();

    pfds = malloc(num_cpus * sizeof(struct pollfd));
    for (i = 0; i < num_cpus; i++) {
        snprintf(trace_path, sizeof(trace_path), "%s/block/%s/trace%d",
                DEBUG_FS_PATH, buts.name, i);
        fprintf(stderr, "%s\n", trace_path);

        if ((trace_fd = open(trace_path, O_RDONLY)) == -1)
            perror("open");

        pfds[i].fd = trace_fd;
        pfds[i].events = POLLIN;
    }

    buf = malloc(READ_BUF_SIZE);

    while (total_read_bytes < READ_BUF_SIZE) {
        if (poll(pfds, num_cpus, 1000) < 0)
            perror("poll");

        fprintf(stderr, "Poll returned\n");

        for (i = 0; i < num_cpus; i++) {
            trace_fd = pfds[i].fd;

            if ((read_bytes = read(trace_fd, buf, READ_BUF_SIZE)) == (size_t)(-1)) {
                perror("read");
                break;
            }
            if (read_bytes == 0) {
                fprintf(stderr, "%s\n", "End of file.");
                break;
            } else {
                fprintf(stderr, "Read %ld bytes\n", read_bytes);
                if (write(1, buf, read_bytes) == (ssize_t)(-1)) {
                    perror("write");
                    break;
                }
            }

            total_read_bytes += read_bytes;
        }
    }

    for (i = 0; i < num_cpus; i++) {
        trace_fd = pfds[i].fd;
        if (trace_fd != -1) {
            if (close(trace_fd)) {
                perror("close trace_fd");
            }
        }
    }

    free(buf);
    free(pfds);

	if (ioctl(dev_fd, BLKTRACESTOP))
		perror("ioctl: BLKTRACESTOP");

	if (ioctl(dev_fd, BLKTRACETEARDOWN))
		perror("ioctl: BLKTRACETEARDOWN");

    if (dev_fd != -1) {
        if (close(dev_fd)) {
            perror("close dev_fd");
        }
    }
	
	return 0;
}
