#include <glib.h>
#include <stdbool.h>

#include "tracecontroller.h"
#include "trace.h"

static GHashTable *control_table = NULL;

/* We are lazy with this mutex, we simply lock it at the beginning of
 * these functions and release it at the end. */
G_LOCK_DEFINE_STATIC(tc_table_mutex);

G_LOCK_DEFINE(term_cond_mutex);
GCond *term_cond;

typedef struct _trace_control_thread {
	GThread *thread;
	bool should_term;
} trace_control_thread;

static void trace_controller_run(gpointer data, gpointer user_data);

void initialize_tracing()
{
	G_LOCK(tc_table_mutex);
	if (control_table == NULL) {
		control_table = g_hash_table_new(g_str_hash, NULL);
		term_cond = g_cond_new();
	}
	G_UNLOCK(tc_table_mutex);
}

void start_tracer(const char *path)
{
	G_LOCK(tc_table_mutex);

	if (!g_hash_table_lookup(control_table, path)) {
		trace_control_thread *tct = g_new(trace_control_thread, 1);
		tct->should_term = false;
		/* args: func, data, joinable, GError location */
		tct->thread = g_thread_create(NULL, term_cond, TRUE, NULL);
	}

	G_UNLOCK(tc_table_mutex);
}

void end_tracer(const char *path)
{
	G_LOCK(tc_table_mutex);

	trace_control_thread *tct;

	if ((tct = g_hash_table_lookup(control_table, path))) {
		G_LOCK(term_cond_mutex);

		tct->should_term = true;
		g_cond_broadcast(term_cond);

		G_UNLOCK(term_cond_mutex);
		
		g_thread_join(tct->thread);
		g_hash_table_remove(control_table, path);
		g_free(tct);
	}

	G_UNLOCK(tc_table_mutex);
}
