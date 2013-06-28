#ifndef	_RANGETREE_H
#define	_RANGETREE_H

#include "rbtree.h"
#include <unistd.h>

/* start is inclusive, end is exclusive */
struct range {
	long start;
	struct rb_node range_node;
	long end;
};

extern struct range *add_value(long, struct rb_root *);

#endif
