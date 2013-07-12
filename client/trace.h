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
	struct trace_linked_list *prev;
	struct trace_tree *tree;
	struct trace_linked_list *next;
};

static struct trace_linked_list *head = NULL;

//static MUTEX = STATIC ??

/* Returns NULL if unable to find tt in linked list
 * else returns the removed pointer to the trace_tree */
static inline struct trace_tree *remove_trace(struct trace_tree *tt)
{
	struct trace_tree *removed_trace = NULL;
	struct trace_linked_list *tt_node = head;

	/* TODO: Prevent cancel */
	/* TODO: Get mutex */

	if (head == NULL) {
		goto release;
	}
	
	/* Find the node containing tt. After this, tt_node will point to tt */
	while (tt_node->tree != tt) {
		/* We failed to find it and should return */
		if (tt_node->next == head) {
			goto release;
		}
		tt_node = tt_node->next;
	}

	/* Move head if needed */
	if (head == tt_node) {
		if (head->next == head)
			head = NULL;
		else
			head = head->next;
	}

	/* Remove tt_node from the list */
	(tt_node->prev)->next = tt_node->next;
	(tt_node->next)->prev = tt_node->prev;
	free(tt_node);

	removed_trace = tt;
release:
	/* TODO: Release mutex */
	/* TODO: Enable cancel */

	return removed_trace;
}

static inline void add_trace(struct trace_tree *tt)
{
	struct trace_linked_list *tt_node = NULL;
	/* TODO: Prevent cancel */
	/* TODO: Get mutex */


	tt_node = malloc(sizeof(struct trace_linked_list));
	if (tt_node == NULL) {
		syslog(LOG_ERR, "Unable to malloc tt_node: %m");
		goto release;
	}

	tt_node->tree = tt;

	if (head == NULL) {
		head = tt_node;
		head->next = head;
		head->prev = head;
	} else {
		(head->prev)->next = tt_node;
		(head->next)->prev = tt_node;
	}

release:
	/* TODO: Release mutex */
	/* TODO: Enable cancel */
	return;
}

#endif
