CC=clang
CFLAGS=-g -W -Wall -D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 \
	$(shell pkg-config --cflags glib-2.0 gthread-2.0)
LDLIBS=-lrt -lpthread $(shell pkg-config --libs glib-2.0 gthread-2.0)

dattod: dattod.o trace.o rangetree.o rbtree.o tracecontroller.o

dattod.o: dattod.c dattod.h tracecontroller.h

tracecontroller.o: tracecontroller.c tracecontroller.h

trace.o: trace.c trace.h

rangetree.o: rangetree.c rangetree.h

rbtree.o: rbtree.c rbtree.h

clean:
	rm -f trace.o tracetable.o rangetree.o rbtree.o dattod.o dattod
