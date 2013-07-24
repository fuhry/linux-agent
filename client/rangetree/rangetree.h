#ifndef	_RANGETREE_H
#define	_RANGETREE_H

#include "rbtree.h"
#include <unistd.h>

typedef struct rb_root range_tree;

/* start is inclusive, end is exclusive */
struct range {
	long start;
	struct rb_node range_node;
	long end;
};

typedef struct rb_node ** range_iter;

extern range_iter rt_get_iter(range_tree *);
extern struct range *rt_next_range(range_iter);

/* Add a single value to the heap-allocated range tree.
 *
 * If the value is already in the tree, the range it is
 * in is returned. Otherwise, the new range is returned.
 *
 * If two ranges overlap after the addition, the ranges
 * are merged.
 */
extern struct range *rt_add_value(range_tree *, long);

/* Call this function to free all nodes in the range tree */
extern void rt_free_tree(range_tree *);

#endif
