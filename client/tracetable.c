#include <glib.h>
#include <stdint.h>
#include "tracetable.h"
#include "rangetree.h"

static GHashTable *trace_table = NULL;
G_LOCK_DEFINE_STATIC(trace_table);

static void _add_block(gpointer tree, gpointer value)
{
	rt_add_value(tree, *(long *)value);
}

static void _free_tree(gpointer tree, gpointer unused)
{
	rt_free_tree(tree);
}

void tr_initialize()
{
	G_LOCK(trace_table);
	if (trace_table == NULL) {
		trace_table = g_hash_table_new(g_str_hash, NULL);
	}
	G_UNLOCK(trace_table);
}

void tr_new_tree(const char *path)
{
	G_LOCK(trace_table);

	if (g_hash_table_lookup(trace_table, path) == NULL) {
		range_tree *tree;
		GPtrArray *tree_arr;

		tree_arr = g_ptr_array_new();

		tree = g_new0(range_tree, 1);
		g_ptr_array_add(tree_arr, tree);

		g_hash_table_insert(trace_table, (gpointer)path, tree_arr);
	}

	G_UNLOCK(trace_table);
}

void tr_remove_tree(const char *path)
{
	GPtrArray *tree_arr = NULL;

	G_LOCK(trace_table);

	tree_arr = g_hash_table_lookup(trace_table, path);

	if (tree_arr != NULL) {
		g_ptr_array_foreach(tree_arr, _free_tree, NULL);
		g_ptr_array_free(tree_arr, TRUE);
		g_hash_table_remove(trace_table, path);
	}

	G_UNLOCK(trace_table);
}

void tr_add_block(const char *path, uint64_t block)
{
	GPtrArray *tree_arr;

	G_LOCK(trace_table);
	
	tree_arr = g_hash_table_lookup(trace_table, path);

	if (tree_arr != NULL) {
		g_ptr_array_foreach(tree_arr, _add_block, &block);
	}

	G_UNLOCK(trace_table);
}
void tr_prepare_for_take(const char *path)
{
	GPtrArray *tree_arr;

	G_LOCK(trace_table);
	
	tree_arr = g_hash_table_lookup(trace_table, path);

	if (tree_arr != NULL) {
		range_tree *tree = g_new0(range_tree, 1);

		g_ptr_array_add(tree_arr, tree);
	}

	G_UNLOCK(trace_table);
}

range_tree *tr_take_tree(const gchar *path)
{
	GPtrArray *tree_arr = NULL;
	range_tree *tree = NULL;

	G_LOCK(trace_table);

	tree_arr = g_hash_table_lookup(trace_table, path);

	if (tree_arr != NULL) {
		tree = g_ptr_array_remove_index(tree_arr, 0);
	}

	G_UNLOCK(trace_table);

	return tree;
}
