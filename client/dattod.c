#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <utime.h>

#include "dattod.h"

#define MAX_ERR_LEN 100
/* 20 characters is more than enough for the 4 million max specified in 
 * linux/threads.h PID_MAX_LIMIT */
#define MAX_PID_LEN 20

void err_log(char *);

void cleanup();
int get_lock();
void setup_exit_handler();
void end_handler(int);

/* done is set when a kill signal is caught */
static volatile sig_atomic_t done;

/* got_msg is set when a message is available on the message queue */
static volatile sig_atomic_t got_msg;

/* reload is set when we should reload the configuration file */
static volatile sig_atomic_t reload;

int main(int argc, char **argv) {
	mqd_t mqd;
	int lockfd;
	int num_wrote;
	sigset_t block_mask;

	if (argc > 1) {
		fprintf(stderr, "usage: %s\n", argv[0]);
		return 1;
	}

	openlog(NULL, LOG_PERROR, 0);

	if ((lockfd = get_lock()) == -1) {
		return 1;
	}

	setup_exit_handler();

	/* TODO: Move to function and improve error checking.
	 * In particular, check if the queue exists and remove it */
	mqd = mq_open(MQUEUE_PATH, O_RDONLY | O_CREAT | O_EXCL,
			S_IRUSR | S_IWUSR, NULL);
	if (mqd == (mqd_t)(-1)) {
		err_log("Error opening message queue");
		return 1;
	}

	/* Become a daemon. This backgrounds the process, cds to /, and
	 * redirects std{in,out,err} to /dev/null, among other things */
	daemon(0, 0);

	/* Write our (newly acquired) PID to the lock file */
	if (write_pid(lock_fd)) {
		goto out;
	}

	sigemptyset(&block_mask);

	/* Stop signals */
	sigaddset(&block_mask, SIGINT);
	sigaddset(&block_mask, SIGTERM);

	/* Reload configuration */
	sigaddset(&block_mask, SIGHUP);

	/* Got message on message queue */
	sigaddset(&block_mask, SIGUSR1);

	while (!done) {
		utime("/tmp/dattod", NULL);
		sleep(1);
	}

out:
	/* If these fail we can't do much about it, but should log it */

	if (mq_close(mqd)) {
		err_log("Unable to close message queue descriptor");
	}

	if (mq_unlink(MQUEUE_PATH)) {
		err_log("Unable to remove message queue descriptor");
	}

	if (close(lockfd)) {
		err_log("Unable to close lock file descriptor");
	}

	return 0;
}

/*
 * Acquire the PID lock to prevent multiple instances of dattod from running
 * at once.
 * Returns the positive lock fd on success and -1 on failure.
 */
int get_lock() {
	int lockfd;
	int err;
	int n_read;
	char pid[MAX_PID_LEN];
	char err_msg[MAX_ERR_LEN];

	lockfd = open(LOCK_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (lockfd == -1) {
		err_log("Unable to open lock file");
		return -1;
	}

	if (flock(lockfd, LOCK_EX | LOCK_NB)) {
		err = errno;
		if (err == EWOULDBLOCK) {
			n_read = read(lockfd, pid, MAX_PID_LEN - 1);
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
		close(lockfd);
		return -1;
	}

	return lockfd;

}

/*
 * Write our PID to the lock file.
 * Returns 0 on success and -1 on failure
 */
int write_pid(int lockfd) {
	if (ftruncate(lockfd, 0)) {
		err_log("Unable to truncate PID file");
		return -1
	}

	num_wrote = dprintf(lockfd, "%d", getpid());
	if (num_wrote < 0) {
		err_log("Unable to write PID to lock file");
		return -1
	}

	return 0;
}

void setup_exit_handler() {
	struct sigaction sa;
	sigset_t block_all;
	
	/* Block all other signals while handling exit signals */
	sigfillset(&block_all);
	sa.sa_handler = end_handler;
	sa.sa_mask = block_all;

	/* Intentionally exclude SIGQUIT as we want to exit without cleaning
	 * up on a SIGQUIT to help with debugging */

	if (sigaction(SIGTERM, &sa, NULL)) {
		err_log("Unable to set SIGTERM sigaction");
		return;
	}
	if (sigaction(SIGINT, &sa, NULL)) {
		err_log("Unable to set SIGINT sigaction");
		return;
	}
}

/* Perform clean up at exit. We don't bother to close fds here as they
 * will be automatically closed on exit and any errors that might be reported
 * at close time can't be resolved at this point. */
void end_handler(int signum) {
	/* unused */
	(void)signum;

	done = 1;
}

void err_log(char *log_msg) {
	if (errno) {
		syslog(LOG_ERR, "%s - %m\n", log_msg);
	} else {
		syslog(LOG_ERR, "%s\n", log_msg);
	}
}
