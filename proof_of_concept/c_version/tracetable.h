#include <glib.h>
#include <stdint.h>
#include "rangetree.h"

void tr_initialize();

void tr_new_tree(const char *path);
void tr_remove_tree(const char *path);

void tr_add_block(const char *path, uint64_t block);

void tr_prepare_for_take(const char *path);
range_tree *tr_take_tree(const char *path);
