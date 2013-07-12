CC=gcc
DEBUG=-g
CFLAGS=-D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
LIBS=-lrt -lpthread

dattod: trace.o rangetree.o rbtree.o
	$(CC) $(CFLAGS) $(DEBUG) -W -Wall -o dattod dattod.c trace.o rangetree/rangetree.o rangetree/rbtree.o $(LIBS) 

trace.o:
	$(CC) $(CFLAGS) $(DEBUG) -W -Wall -c -o trace.o trace.c $(LIBS)

rangetree.o:
	$(CC) $(CFLAGS) $(DEBUG) -W -Wall -c -o rangetree/rangetree.o ./rangetree/rangetree.c $(LIBS)

rbtree.o:
	$(CC) $(CFLAGS) $(DEBUG) -W -Wall -c -o rangetree/rbtree.o ./rangetree/rbtree.c $(LIBS)

clean:
	rm -f trace.o rangetree/rangetree.o rangetree/rbtree.o dattod 
