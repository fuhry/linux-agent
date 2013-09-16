#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <syslog.h>
#include <unistd.h>
#include <utime.h>
#include <sys/time.h>

#include <glib.h>

#include "dattod.h"
#include "tracecontroller.h"

/* 20 characters is more than enough for the 4 million max (7 characters)
 * specified in linux/threads.h PID_MAX_LIMIT */
#define MAX_PID_LEN 20

static int _get_lock();
static int _write_pid(int);

static void _start_trace_controllers(char **);

/* Listener thread */
GThread *_listener_thread;
static void _start_mq_listener();
static gpointer _handle_mq(gpointer);

/* _done is set when a kill signal is caught */
static volatile sig_atomic_t _done = 0;
/* _got_msg is set when a message is available on the message queue */
static volatile sig_atomic_t _got_msg = 0;

int main(int argc, char **argv)
{
	int lock_fd;
	sigset_t block_mask;
	sigset_t orig_mask;
	int foreground = 0;
	int sig;

	char *dev_paths[] = {"/dev/sda", "/dev/sdb", NULL};

	if (argc > 1) {
		/* --fg puts dattod in the foreground for debugging */
		if (strcmp(argv[1], "--fg") == 0) {
			foreground = 1;
		} else {
			fprintf(stderr, "usage: %s [--fg]\n", argv[0]);
			return 1;
		}
	}

	openlog(NULL, LOG_PERROR, 0);

	if ((lock_fd = _get_lock()) == -1) {
		return 1;
	}
	
	/* TODO: Parse a config file */

	/* Become a daemon. This backgrounds the process, cds to /, and
	 * redirects std{in,out,err} to /dev/null, among other things */
	if (!foreground) {
		daemon(0, 0);
	} else {
		syslog(LOG_DEBUG, "Starting dattod\n");
	}

	/* Write our (newly acquired) PID to the lock file */
	if (_write_pid(lock_fd)) {
		return 1;
	}


	/* Block various signals so child threads don't handle them.
	 * - SIGINT / SIGTERM so we can clean up before exiting
	 * - SIGHUP so the main thread can reload the config
	 */
	sigemptyset(&block_mask);
	sigaddset(&block_mask, SIGINT);
	sigaddset(&block_mask, SIGTERM);
	sigaddset(&block_mask, SIGHUP);

	if (pthread_sigmask(SIG_BLOCK, &block_mask, &orig_mask)) {
		syslog(LOG_ERR, "Error blocking signals");
		return 1;
	}

	/* TODO: Add a debug option that supports
	 * g_thread_init_with_errorcheck_mutexes */
	g_thread_init(NULL);

	_start_trace_controllers(dev_paths);
	_start_mq_listener();

	while (!_done) {
		if (sigwait(&block_mask, &sig)) {
			syslog(LOG_ERR, "sigwait");
			break;
		}
		switch (sig) {
			case SIGTERM:
			case SIGINT:
				_done = 1;
				break;
			case SIGHUP:
				fprintf(stderr, "SIGHUP\n");
				break;
			default:
				fprintf(stderr, "Got unxpected signal:"
						" %d\n", sig);
				break;
		}
	}

	g_thread_join(_listener_thread);
	return 0;
}

static void _start_mq_listener()
{
	/* Start message queue listener thread */
	_listener_thread = g_thread_create(_handle_mq, NULL, true, NULL);
}

/*
 * This should only be run from the mq_listener thread
 */
static gpointer _handle_mq(gpointer arg)
{
	ssize_t num_read;
	char *buf = NULL;

	int msg_type;

	struct timespec mq_timeout = { .tv_sec = 2, .tv_nsec = 0 };

	mqd_t mqd;

	/* Unused */
	(void)arg;

	mqd = mq_open(MQUEUE_PATH, O_RDONLY | O_CREAT | O_EXCL,
			S_IRUSR | S_IWUSR, &msg_q_attr);

	if (mqd == (mqd_t)(-1)) {
		syslog(LOG_ERR, "Error opening message queue");
		_done = true;
		return NULL;
	}


	buf = g_malloc(msg_q_attr.mq_msgsize);

	while (!_done) {
		num_read = mq_timedreceive(mqd, buf,
			msg_q_attr.mq_msgsize, NULL, &mq_timeout);
		if (num_read == -1 && errno == ETIMEDOUT) {
			continue;
		} else if (num_read == -1) {
			syslog(LOG_ERR, "mq_receive: %m");
			break;
		} else if (num_read == 0) {
			/* Messages shouldn't be zero length..
			 * but deal if they are */
			continue;
		}

		msg_type = buf[0];

		switch (msg_type) {
			case ECHO:
				if (num_read < msg_q_attr.mq_msgsize) {
					buf[num_read] = '\0';
					printf("%s\n", buf+1);
				}
				break;
			default:
				printf("got type: %d\n", msg_type);
				break;
		}
	}

	g_free(buf);

	/* If this fails, we can't do anything about it but should log */
	if (mq_unlink(MQUEUE_PATH)) {
		syslog(LOG_ERR, "Unable to remove message"
				" queue descriptor: %m");
	}

	/* If we get here then something went wrong */
	return NULL;
}

/* dev_paths is a list of paths, terminated with a null pointer */
static void _start_trace_controllers(char **dev_paths)
{
	int i;
	int stat_err;
	struct stat s;

	initialize_tracing();
	for (i = 0; dev_paths[i] != NULL; i++) {
		/* Only trace it if it is a block device */
		stat_err = stat(dev_paths[i], &s);
		/* TODO: Check if symlinks work here
		 *       Desired behavior is that they should */
		if (!stat_err && S_ISBLK(s.st_mode)) {
			/* TODO: Reenable */
			/* start_tracer(dev_paths[i]); */
		} else {
			syslog(LOG_ERR, "Bad path (%m): %s", dev_paths[i]);
		}
	}
}


/*
 * Acquire the PID lock to prevent multiple instances of dattod from running
 * at once.
 * Returns the positive lock fd on success and -1 on failure.
 */
static int _get_lock()
{
	int lock_fd;
	int n_read;
	char pid[MAX_PID_LEN];

	lock_fd = open(LOCK_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (lock_fd == -1) {
		syslog(LOG_ERR, "Unable to open lock file");
		return -1;
	}

	if (flock(lock_fd, LOCK_EX | LOCK_NB)) {
		if (errno == EWOULDBLOCK) {
			n_read = read(lock_fd, pid, MAX_PID_LEN - 1);
			if (n_read < 1) {
				syslog(LOG_ERR, "Unable to get lock pid");
			} else {
				syslog(LOG_ERR, "Already running with" 
						" PID: %s", pid);
			}
		} else {
			syslog(LOG_ERR, "Unable to acquire lock");

		}
		/* No need to check return */
		close(lock_fd);
		return -1;
	}

	return lock_fd;

}

/*
 * Write our PID to the lock file.
 * Returns 0 on success and -1 on failure
 */
static int _write_pid(int lock_fd)
{
	int num_wrote;

	if (ftruncate(lock_fd, 0)) {
		syslog(LOG_ERR, "Unable to truncate PID file");
		return -1;
	}

	num_wrote = dprintf(lock_fd, "%d", getpid());
	if (num_wrote < 0) {
		syslog(LOG_ERR, "Unable to write PID to lock file");
		return -1;
	}

	return 0;
}
