#ifndef TRACE_H
#define TRACE_H

#include "rangetree/rangetree.h"

struct trace_tree {
	char *block_dev_path;
	struct rb_root **range_tree;
	pthread_mutex_t *mutex;
};

#endif
