#include <glib.h>
#include "rangetree/rangetree.h"

void tr_initialize();

void tr_new_tree(const gchar *path);
void tr_remove_tree(const gchar *path);

void tr_add_block(const gchar *path, guint64 block);

void tr_prepare_for_take(const gchar *path);
range_tree *tr_take_tree(const gchar *path);
