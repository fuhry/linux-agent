#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <utime.h>
#include <sys/time.h>

#include "dattod.h"

#define MAX_ERR_LEN 100
/* 20 characters is more than enough for the 4 million max specified in 
 * linux/threads.h PID_MAX_LIMIT */
#define MAX_PID_LEN 20

void err_log(char *);

static int _get_lock();
static int _write_pid(int);

/* Thread functions */
void *_handle_mq(void *);

/* _done is set when a kill signal is caught */
static volatile sig_atomic_t _done = 0;
/* _got_msg is set when a message is available on the message queue */
static volatile sig_atomic_t _got_msg = 0;

int main(int argc, char **argv)
{
	mqd_t mqd;
	int lock_fd;
	sigset_t block_mask;
	sigset_t orig_mask;
	int foreground = 0;
	int sig;

	if (argc > 1) {
		/* -f puts dattod in the foreground for debugging */
		if (strcmp(argv[1], "-f") == 0) {
			foreground = 1;
		} else {
			fprintf(stderr, "usage: %s\n", argv[0]);
			return 1;
		}
	}

	openlog(NULL, LOG_PERROR, 0);

	if ((lock_fd = _get_lock()) == -1) {
		return 1;
	}

	/* TODO: Move to function and improve error checking.
	 * In particular, check if the queue exists and remove it */
	mqd = mq_open(MQUEUE_PATH, O_RDONLY | O_CREAT | O_EXCL,
			S_IRUSR | S_IWUSR, &msg_q_attr);
	if (mqd == (mqd_t)(-1)) {
		err_log("Error opening message queue");
		return 1;
	}

	/* Become a daemon. This backgrounds the process, cds to /, and
	 * redirects std{in,out,err} to /dev/null, among other things */
	if (!foreground) {
		daemon(0, 0);
	}

	/* Write our (newly acquired) PID to the lock file */
	if (_write_pid(lock_fd)) {
		goto out;
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
		err_log("Error blocking signals");
		goto out;
	}

	/* TODO: Start block trace thread */
	/* TODO: Start message listener thread */

	while (!_done) {
		if (sigwait(&block_mask, &sig)) {
			err_log("sigwait");
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
				fprintf(stderr, "Got unxpected signal: %d\n", sig);
		}
	}

out:
	/* If these fail we can't do much about it, but should log it */

	if (mq_unlink(MQUEUE_PATH)) {
		err_log("Unable to remove message queue descriptor");
	}

	return 0;
}

/*
 * This should only be run from a child process
 */
/* TODO make static */
void *_handle_mq(void *arg)
{
	mqd_t mqd = *(mqd_t*)(arg);
	ssize_t num_read;
	char *buf = NULL;

	int msg_type;

	buf = malloc(msg_q_attr.mq_msgsize);
	if (buf == NULL) {
		err_log("malloc(buf)");
		return NULL;
	}

	while ((num_read = mq_receive(mqd, buf,
					msg_q_attr.mq_msgsize, NULL)) >= 0) {

		/* Messages shouldn't be zero length.. but deal if they are */
		if (num_read == 0) {
			continue;
		}

		msg_type = buf[0];

		switch (msg_type) {
			case ECHO:
				buf[num_read - 1] = '\0';
				break;
			case FULL:
				printf("sleeping for %ds\n", buf[1]);
				sleep(buf[1]);
				break;
			default:
				printf("got type: %d\n", msg_type);
				break;
		}
	}

	/* If we get here then something went wrong */
	err_log("mq_receive");

	return NULL;
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
	char err_msg[MAX_ERR_LEN];

	lock_fd = open(LOCK_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (lock_fd == -1) {
		err_log("Unable to open lock file");
		return -1;
	}

	if (flock(lock_fd, LOCK_EX | LOCK_NB)) {
		if (errno == EWOULDBLOCK) {
			n_read = read(lock_fd, pid, MAX_PID_LEN - 1);
			if (n_read < 1) {
				err_log("Unable to get lock pid");
			} else {
				pid[n_read] = '\0';
				snprintf(err_msg, MAX_ERR_LEN - 1,
					"Already running with PID: %s", pid);
				err_msg[MAX_ERR_LEN] = '\0';
				err_log(err_msg);
			}
		} else {
			err_log("Unable to acquire lock");

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
		err_log("Unable to truncate PID file");
		return -1;
	}

	num_wrote = dprintf(lock_fd, "%d", getpid());
	if (num_wrote < 0) {
		err_log("Unable to write PID to lock file");
		return -1;
	}

	return 0;
}

void err_log(char *log_msg)
{
	if (errno) {
		syslog(LOG_ERR, "%s - %m\n", log_msg);
	} else {
		syslog(LOG_ERR, "%s\n", log_msg);
	}
}
