extern void initialize_tracing();
extern void start_tracer(const char *);
extern void end_tracer(const char *);

/* All trace controllers will wake on signals to this GCond, but will
 * check a bool specific to that controller */
extern GCond *term_cond;
G_LOCK_EXTERN(term_cond_mutex);
