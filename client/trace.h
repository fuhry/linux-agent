#ifndef TRACE_H
#define TRACE_H

#include "rangetree/rangetree.h"

struct trace_tree {
	char *block_dev_path;
	struct rb_root **range_tree;
	/* This mutex is for the trace_tree struct, not just the range_tree */
	pthread_mutex_t *mutex;
};

struct trace_linked_list {
	struct trace_tree *tree;
	struct trace_linked_list *next;
};

static struct trace_linked_list *head = NULL;

//static MUTEX = STATIC ??

/* Returns 0 if removed successfully, non-zero otherwise */
static inline struct trace_tree *remove_trace(struct trace_tree *tt)
{
	struct trace_tree *tree = NULL;
	struct trace_linked_list *to_free = NULL;
	struct trace_linked_list **curr = &head;

	/* TODO: Prevent cancel */
	/* TODO: Get mutex */

	while (*curr) {
		tree = (*curr)->tree;
		if (tree == tt) {
			to_free = *curr;

			*curr = (*curr)->next;
			free(to_free);

			break;
		} else {
			curr = &(*curr)->next;
		}
	}

	/* TODO: Release mutex */
	/* TODO: Enable cancel */

	return tree;
}

static inline void add_trace(struct trace_tree *tt)
{
	struct trace_linked_list *tt_node = NULL;
	/* TODO: Prevent cancel */
	/* TODO: Get mutex */


	/* Initialize trace_linked_list struct */
	tt_node = malloc(sizeof(struct trace_linked_list));
	if (tt_node == NULL) {
		syslog(LOG_ERR, "Unable to malloc tt_node: %m");
		goto release;
	}
	tt_node->tree = tt;

	/* Insert at front */
	tt_node->next = head;
	head = tt_node;

release:
	/* TODO: Release mutex */
	/* TODO: Enable cancel */
	return;
}

#endif
