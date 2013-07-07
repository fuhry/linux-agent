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

#define NOTIFY_SIG SIGUSR1

void err_log(char *);

static int get_lock();
static int setup_handlers();
static void set_flag(int);
static int write_pid(int);
static void process_messages(mqd_t);
static void handle_msg(void *, ssize_t);

/* done is set when a kill signal is caught */
static volatile sig_atomic_t done = 0;
/* got_msg is set when a message is available on the message queue */
static volatile sig_atomic_t got_msg = 0;
/* reload is set when we should reload the configuration file */
static volatile sig_atomic_t reload = 0;

static struct sigevent msg_notification =
{
	.sigev_notify = SIGEV_SIGNAL,
	.sigev_signo = NOTIFY_SIG,
};

/* 0 indicates end of array */
const int handled_signals[] = {SIGINT, SIGTERM, NOTIFY_SIG, SIGHUP, 0};

int main(int argc, char **argv) {
	mqd_t mqd;
	int lock_fd;
	sigset_t block_mask;
	sigset_t orig_mask;
	int i;
	int foreground = 0;

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

	if ((lock_fd = get_lock()) == -1) {
		return 1;
	}

	if (setup_handlers() != 0) {
		return 1;
	}

	/* TODO: Move to function and improve error checking.
	 * In particular, check if the queue exists and remove it */
	mqd = mq_open(MQUEUE_PATH, O_RDONLY | O_CREAT | O_EXCL | O_NONBLOCK,
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
	if (write_pid(lock_fd)) {
		goto out;
	}

	/* Block all of our handled signals as we will be using
	 * sigsuspend in the loop below */
	sigemptyset(&block_mask);
	for (i = 0; handled_signals[i] != 0; i++) {
		sigaddset(&block_mask, handled_signals[i]);
	}

	if (sigprocmask(SIG_BLOCK, &block_mask, &orig_mask)) {
		err_log("Error blocking signals");
		goto out;
	}

	if (mq_notify(mqd, &msg_notification)) {
		err_log("mq_notify");
		goto out;
	}

	while (!done) {
		if (sigsuspend(&orig_mask) && errno != EINTR) {
			err_log("sigsuspend");
		}

		if (got_msg) {
			got_msg = 0;
			/* Reregister our message listener */
			if (mq_notify(mqd, &msg_notification)) {
				err_log("mq_notify");
				continue;
			}
			process_messages(mqd);
		}
		if (reload) {
			reload = 0;
			utime("/tmp/reload", NULL);
		}
	}

out:
	/* If these fail we can't do much about it, but should log it */
	if (mq_close(mqd)) {
		err_log("Unable to close message queue descriptor");
	}

	if (mq_unlink(MQUEUE_PATH)) {
		err_log("Unable to remove message queue descriptor");
	}

	if (close(lock_fd)) {
		err_log("Unable to close lock file descriptor");
	}

	return 0;
}

static void process_messages(mqd_t mqd) {
	ssize_t num_read;
	void *buf = NULL;

	buf = malloc(msg_q_attr.mq_msgsize);
	if (buf == NULL) {
		err_log("malloc(buf)");
		return;
	}

	while ((num_read = mq_receive(mqd, buf,
			msg_q_attr.mq_msgsize, NULL)) >= 0) {
		switch(fork()) {
			/* Child */
			case 0:
				mq_close(mqd);
				handle_msg(buf, num_read);
				_exit(0);
				break;
			/* Error */
			case -1:
				err_log("fork");
				break;
			/* Parent */
			default:
				break;
		}
	}

	/* EAGAIN means we reached the end of the messages in the queue */
	if (errno != EAGAIN) {
		err_log("mq_receive");
	}

	free(buf);
}

static void handle_msg(void *buf, ssize_t num_read) {
	/* Debugging code for now */
	((char*)buf)[num_read - 1] = '\0';
	printf("%s\n", (char*)buf);
}


/*
 * Acquire the PID lock to prevent multiple instances of dattod from running
 * at once.
 * Returns the positive lock fd on success and -1 on failure.
 */
static int get_lock() {
	int lock_fd;
	int err;
	int n_read;
	char pid[MAX_PID_LEN];
	char err_msg[MAX_ERR_LEN];

	lock_fd = open(LOCK_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (lock_fd == -1) {
		err_log("Unable to open lock file");
		return -1;
	}

	if (flock(lock_fd, LOCK_EX | LOCK_NB)) {
		err = errno;
		if (err == EWOULDBLOCK) {
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
static int write_pid(int lock_fd) {
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

/*
 * Setup the various signal handlers.
 * Returns 0 on success and 1 on failure
 *
 * This includes:
 *	 SIGTERM    - done
 *	 SIGINT     - done
 *	 NOTIFY_SIG - got_msg
 *	 SIGHUP     - reload
 */
static int setup_handlers() {
	struct sigaction sa;
	sigset_t block_all;
	int i;
	
	/* Block all other signals while handling a signal. This is okay as
	 * our exit handler is very brief */
	sigfillset(&block_all);
	sa.sa_mask = block_all;

	sa.sa_handler = set_flag;

	for (i = 0; handled_signals[i] != 0; i++) {
		if (sigaction(handled_signals[i], &sa, NULL)) {
			err_log("Unable to set sigaction");
			return 1;
		}
	}

	return 0;
}

/* Set flag to indicate we got a terminate signal */
static void set_flag(int signum) {
	switch (signum) {
	/* Intentionally exclude SIGQUIT/SIGABRT/etc. as we want to exit
	 * without cleaning up on a SIGQUIT to help with debugging */
	case SIGTERM:
	case SIGINT:
		done = 1;
		break;
	case NOTIFY_SIG:
		got_msg = 1;
		break;
	case SIGHUP:
		reload = 1;
		break;
	default:
		/* Should be unreachable, but just in case */
		if (signal(signum, SIG_DFL) != SIG_ERR) {
			raise(signum);
		}
	}
}

void err_log(char *log_msg) {
	if (errno) {
		syslog(LOG_ERR, "%s - %m\n", log_msg);
	} else {
		syslog(LOG_ERR, "%s\n", log_msg);
	}
}
