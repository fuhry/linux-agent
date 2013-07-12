#include <errno.h>
#include <fcntl.h>
#include <linux/blktrace_api.h>
#include <linux/fs.h>
#include <linux/limits.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <syslog.h>

#include "trace.h"

#define DEBUG_FS_PATH "/sys/kernel/debug"

void *trace_path(char *path);

static void close_trace(void *);
static void cleanup_tree(void *);
static void cancel_cpu_tracers(void *);
static void *trace_cpu(void *);
static int skip_noseek_fd(int, int);

struct trace_thread {
	pthread_t id;
	struct trace_tree *tree;
	char *bt_name;
	int cpu_num;
};

/* 0 on success, < 0 on failure with errno */
static int skip_noseek_fd(int fd, int num_skip)
{
	char buf[512];
	int read_bytes = -1;
	int total_bytes = 0;

	do {
		read_bytes = read(fd, &buf, num_skip);
		/* If we get an error, return that error */
		if (read_bytes < 0) {
			return read_bytes;
		}
		total_bytes += read_bytes;
	} while (total_bytes != num_skip);

	return 0;
}

static void *trace_cpu(void *arg)
{
	struct trace_thread *thread = (struct trace_thread *)arg;

	int trace_fd;
	char trace_path[PATH_MAX];

	int read_bytes;
	struct blk_io_trace trace;

	int i;
	int sectors_written;

	int old_cancel_type;

	snprintf(trace_path, sizeof(trace_path), "%s/block/%s/trace%d",
			DEBUG_FS_PATH, thread->bt_name, thread->cpu_num);

	if ((trace_fd = open(trace_path, O_RDONLY)) == -1)
		perror("open");

	while ((read_bytes = read(trace_fd, &trace, sizeof(trace))) > 0) {
		/* Sanity check, make sure the blk_io_trace struct has the
		 * right format */
		if (trace.magic != (BLK_IO_TRACE_MAGIC |
					BLK_IO_TRACE_VERSION)) {
			fprintf(stderr, "Error checking magic number\n");
			fprintf(stderr, "  got magic: 0x%x\n", trace.magic);
			break;
		}

		/* Lock mutex and prevent cancelling */
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,
				&old_cancel_type);
		pthread_mutex_lock(thread->tree->mutex);

		/* Add sectors to the tree */
		sectors_written = trace.bytes / 512 + 1;
		for (i = 0; i < sectors_written + 1; i++) {
			rt_add_value(trace.sector + i,
					*(thread->tree->range_tree));
		}

		/* Unlock mutex and reenable cancelling */
		pthread_mutex_unlock(thread->tree->mutex);
		pthread_setcancelstate(old_cancel_type, NULL);

		/* Skip the extra bytes that might be after the trace */
		if (skip_noseek_fd(trace_fd, trace.pdu_len)) {
			syslog(LOG_ERR, "Error reading debugfs "
					"trace file: %m");
			break;
		}
	}

	return NULL;
}

void *trace_path(char *path)
{
	struct blk_user_trace_setup buts;
	struct trace_tree trace_tree;
	int dev_fd;
	struct trace_thread *threads;

	int i;

	struct rb_root *tree;

	int num_cpus = get_nprocs();

	memset(&buts, 0, sizeof(buts));
	buts.buf_size = 1024 * 1024;
	buts.buf_nr = 4;
	buts.act_mask = BLK_TC_WRITE;

	if ((dev_fd = open(path, O_RDONLY | O_NONBLOCK)) == -1) {
		syslog(LOG_ERR, "unable to open %s: %m", path);
		return NULL;
	}

	pthread_cleanup_push(close_trace, &dev_fd);

	if (ioctl(dev_fd, BLKTRACESETUP, &buts)) {
		syslog(LOG_ERR, "ioctl: BLKTRACESETUP %s: %m", path);
		pthread_exit(NULL);
	}

	if (ioctl(dev_fd, BLKTRACESTART)) {
		syslog(LOG_ERR, "ioctl: BLKTRACESTART %s: %m", path);
		pthread_exit(NULL);
	}

	/* Initialize the trace_tree struct */
	trace_tree.block_dev_path = path;

	pthread_mutex_init(trace_tree.mutex, NULL);
	pthread_cleanup_push(cleanup_tree, &trace_tree);

	if ((tree = malloc(sizeof(struct rb_root))) == NULL) {
		syslog(LOG_ERR, "unable to malloc tree: %m");
		pthread_exit(NULL);
	}

	trace_tree.range_tree = &tree;

	threads = malloc(num_cpus * sizeof(struct trace_thread));
	if (threads == NULL) {
		syslog(LOG_ERR, "unable to malloc threads struct: %m");
		pthread_exit(NULL);
	}

	/* Push the clean up before starting them incase this thread
	 * is cancelled before they all start */
	pthread_cleanup_push(cancel_cpu_tracers, &trace_tree);

	/* Start a trace on each CPU */
	for (i = 0; i < num_cpus; i++) {
		if (pthread_create(&threads[i].id, NULL, trace_cpu,
					threads + i)) {
			pthread_exit(NULL);
		}
	}

	/* Let the child threads run until we are cancelled.
	 * pause() is a cancellation point */
	while (1) {
		pause();
	}

	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);

	return NULL;
}

/* Need to make sure this is not run when something else is using the tree */
static void cancel_cpu_tracers(void *arg)
{
	struct trace_thread *threads = (struct trace_thread *)arg;

	int num_cpus = get_nprocs();
	int i;

	for (i = 0; i < num_cpus; i++) {
		if (pthread_cancel(threads[i].id)) {
			syslog(LOG_ERR, "unable to cancel trace thread");
			continue;
		}
		if (pthread_join(threads[i].id, NULL)) {
			syslog(LOG_ERR, "unable to join trace thread");
			continue;
		}
	}

}

/* Need to make sure this is not run when something else is using the tree */
static void cleanup_tree(void *arg)
{
	int err;
	struct trace_tree *tree = (struct trace_tree *)arg;

	if ((err = pthread_mutex_destroy(tree->mutex))) {
		syslog(LOG_ERR, "could not destroy mutex: %s", strerror(err));
	}

	rt_free_tree(*tree->range_tree);
}

static void close_trace(void *arg)
{
	int dev_fd = *(int *)arg;
	if (ioctl(dev_fd, BLKTRACESTOP))
		perror("ioctl: BLKTRACESTOP");

	if (ioctl(dev_fd, BLKTRACETEARDOWN))
		perror("ioctl: BLKTRACETEARDOWN");

	if (dev_fd != -1) {
		if (close(dev_fd)) {
			perror("close dev_fd");
		}
	}
}
