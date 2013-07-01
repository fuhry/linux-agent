#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <linux/blktrace_api.h>
#include <linux/fs.h>
#include <linux/limits.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#define DEBUG_FS_PATH "/sys/kernel/debug"
#define READ_BUF_SIZE 1024
#define BLOCK_TIME_MILLIS 10

int num_cpus;
int dev_fd = -1;
struct pollfd *pfds;
void *buf;
void cleanup(int signum);

int main(int argc, char **argv) {
	int trace_fd;
	int poll_ret;
	size_t read_bytes;
	size_t total_read_bytes = 0;
	char trace_path[PATH_MAX];
	struct blk_user_trace_setup buts;
	int i;

	if (argc != 2) {
		fprintf(stderr, "usage: %s /dev/<block_dev>\n", argv[0]);
		return 1;
	}

	signal(SIGTERM, cleanup);
	signal(SIGINT, cleanup);

	num_cpus = get_nprocs();

	if ((dev_fd = open(argv[1], O_RDONLY | O_NONBLOCK)) == -1) {
		perror("open");
		return 1;
	}

	memset(&buts, 0, sizeof(buts));
	buts.buf_size = 1024 * 1024;
	buts.buf_nr = 4;
	buts.act_mask = BLK_TC_WRITE;

	if (ioctl(dev_fd, BLKTRACESETUP, &buts))
		perror("ioctl: BLKTRACESETUP");

	if (ioctl(dev_fd, BLKTRACESTART))
		perror("ioctl: BLKTRACESTART");

	pfds = malloc(num_cpus * sizeof(struct pollfd));
	for (i = 0; i < num_cpus; i++) {
		snprintf(trace_path, sizeof(trace_path), "%s/block/%s/trace%d",
				DEBUG_FS_PATH, buts.name, i);
		fprintf(stderr, "%s\n", trace_path);

		if ((trace_fd = open(trace_path, O_RDONLY | O_NONBLOCK)) == -1)
			perror("open");

		pfds[i].fd = trace_fd;
		pfds[i].events = POLLIN;
	}

	buf = malloc(READ_BUF_SIZE);

	while (1) {
		poll_ret = poll(pfds, num_cpus, BLOCK_TIME_MILLIS);
		if (poll_ret < 0)
			perror("poll");

		for (i = 0; i < num_cpus; i++) {
			trace_fd = pfds[i].fd;

			read_bytes = read(trace_fd, buf, READ_BUF_SIZE);

			if (read_bytes == (size_t)(-1)) {
				perror("read");
			}

			if (read_bytes > 0) {
				fprintf(stderr, "Read %ld bytes\n",
					(long int)read_bytes);
			}

			total_read_bytes += read_bytes;
		}
	}
	return 1;
}

void cleanup(int signum) {
	int trace_fd;
	int i;

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

	signal(signum, SIG_DFL);
	raise(signum);
}
