CC=gcc
DEBUG=-g
CFLAGS=-D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
LIBS=-lrt -lpthread

dattod: dattod.c trace.o rangetree.o rbtree.o dattod.h
	$(CC) $(CFLAGS) $(DEBUG) -W -Wall -o dattod dattod.c trace.o rangetree.o rbtree.o $(LIBS) 

trace.o: trace.c trace.h
	$(CC) $(CFLAGS) $(DEBUG) -W -Wall -c trace.c $(LIBS)

rangetree.o: rangetree/rangetree.c rangetree/rangetree.h
	$(CC) $(CFLAGS) $(DEBUG) -W -Wall -c ./rangetree/rangetree.c $(LIBS)

rbtree.o: rangetree/rbtree.c rangetree/rbtree.h
	$(CC) $(CFLAGS) $(DEBUG) -W -Wall -c ./rangetree/rbtree.c $(LIBS)

clean:
	rm -f trace.o rangetree.o rbtree.o dattod 
