#include "rangetree.h"

struct range *add_value(long value, struct rb_root *root)
{
	struct rb_node **curnode = &root->rb_node;
	struct rb_node *nextnode = NULL;
	struct rb_node *parent = NULL;
	struct range *range;
	struct range *nextrange;

	while (*curnode) {
		parent = *curnode;
		range = rb_entry(*curnode, struct range, range_node);

		if (value < range->start) {
			curnode = &(*curnode)->rb_left;
		} else if (value > range->end) {
			curnode = &(*curnode)->rb_right;
		/* In the scenario where the current node has the value to add as
		 * it's (exclusive) end offset, we simply increment the end point of
		 * the current node. */
		} else if (value == range->end) {
			range->end++;
			break;
		/* The value already exists in a range so return */
		} else {
			return range;
		}
	}

	/* If we left the above loop at a NULL leaf, then the curnode pointer is
	 * where we should insert a new node consisting of a single value */
	if (*curnode == NULL) {
		range = malloc(sizeof(struct range));
		range->start = value;
		range->end = value + 1;
		rb_link_node(&range->range_node, parent, curnode);
		rb_insert_color(&range->range_node, root);
	}

	/* As we've made a modification, check for overlapping ranges.
	 * Endpoints will only overlap by one as we've only added a single
	 * value */
	nextnode = rb_next(*curnode);
	if (nextnode) {
		nextrange = rb_entry(nextnode, struct range, range_node);
		if (range->end == nextrange->start) {
			rb_erase(nextnode, root);
			range->end = nextrange->end;
			free(nextrange);
		}
	}

	return range;
}
