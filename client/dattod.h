#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#include <sys/file.h>
#include <fcntl.h>

#define MQUEUE_PATH "/dattod"
#define LOCK_FILE "/var/run/dattod.pid"

const struct mq_attr msg_q_attr =
{
	.mq_maxmsg = 5,
	.mq_msgsize = 1024,
};
