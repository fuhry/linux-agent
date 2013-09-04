#ifndef TRACE_H
#define TRACE_H

#include <syslog.h>

#include "rangetree.h"

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

/* Returns tt if removed successfully, NULL otherwise */
static inline struct trace_tree *remove_trace(struct trace_linked_list **head,
		struct trace_tree *tt)
{
	struct trace_linked_list *to_free = NULL;
	struct trace_linked_list **curr = head;

	while (*curr) {
		if ((*curr)->tree == tt) {
			to_free = *curr;

			*curr = (*curr)->next;
			free(to_free);

			return tt;
		} else {
			curr = &(*curr)->next;
		}
	}

	return NULL;
}

static inline void add_trace(struct trace_linked_list **head,
		struct trace_tree *tt)
{
	struct trace_linked_list *tt_node = NULL;

	/* Initialize trace_linked_list struct */
	tt_node = malloc(sizeof(struct trace_linked_list));
	if (tt_node == NULL) {
		syslog(LOG_ERR, "Unable to malloc tt_node: %m");
		return;
	}
	tt_node->tree = tt;

	/* Insert at front */
	tt_node->next = *head;
	*head = tt_node;

	return;
}

#endif
