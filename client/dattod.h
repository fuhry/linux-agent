#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#include <sys/file.h>
#include <fcntl.h>

#define MQUEUE_PATH "/dattod"
#define LOCK_FILE "/var/run/dattod.pid"

enum msg_types_t {
	ECHO = 'E',
	FULL = 'F',
	INCREMENTAL = 'I',
	STOP = 'S',
} msg_types;

const struct mq_attr msg_q_attr =
{
	.mq_maxmsg = 5,
	.mq_msgsize = 1024,
};
