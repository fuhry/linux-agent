#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#include <sys/file.h>
#include <fcntl.h>

#define MQUEUE_PATH "/dattod"
#define LOCK_FILE "/var/run/dattod.pid"
