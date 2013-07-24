#include <assert.h>
#include <glib.h>
#include <stdio.h>

#include "../tracetable.h"

static void _check_range(struct range *range, long start, long end)
{
	if (range == NULL) {
		printf("Null range: %ld\n", range->start);
	}
	if (range->start != start) {
		printf("Bad start: %ld\n", range->start);
	}
	if (range->end != end) {
		printf("Bad end: %ld\n", range->end);
	}
}

int main()
{
	range_tree *tree;
	range_iter iter;
	struct range *range;

	tr_initialize();

	tr_new_tree("t1");
	tr_new_tree("t2");
	
	/* F: [123, 124) */
	tr_add_block("t1", 123);

	/* F: [123, 124), [125, 126) */
	tr_add_block("t1", 125);

	tr_prepare_for_take("t1");
	/* F: [123, 126) */
	/* S: [124, 125) */
	tr_add_block("t1", 124);

	tree = tr_take_tree("t1");

	/* S: [124, 125), [130, 131) */
	tr_add_block("t1", 130);

	iter = rt_get_iter(tree);
	g_assert(iter != NULL);

	range = rt_next_range(iter);
	g_assert(range != NULL);
	_check_range(range, 123, 126);

	range = rt_next_range(iter);
	g_assert(range == NULL);

	printf("Finished checking first tree\n");

	tr_prepare_for_take("t1");

	/* S: [124, 125), [130, 132) */
	tr_add_block("t1", 131);
	tree = tr_take_tree("t1");

	tr_add_block("t1", 150);

	iter = rt_get_iter(tree);
	g_assert(iter != NULL);

	range = rt_next_range(iter);
	g_assert(range != NULL);
	_check_range(range, 124, 125);

	range = rt_next_range(iter);
	g_assert(range != NULL);
	_check_range(range, 130, 132);

	range = rt_next_range(iter);
	g_assert(range == NULL);

	printf("Finished checking second tree\n");

	return 0;
}
