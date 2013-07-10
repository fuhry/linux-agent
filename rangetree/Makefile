CC=gcc
DEBUG=-g
CFLAGS=-D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

buildtree: buildtree.c rangetree.o rbtree.o
	$(CC) $(CFLAGS) $(DEBUG) -W -Wall -o buildtree buildtree.c rangetree.o rbtree.o $(LIBS)

rangetree.o: rangetree.c rbtree.o
	$(CC) $(CFLAGS) $(DEBUG) -c rangetree.c $(LIBS)

rbtree.o: rbtree.c
	$(CC) $(CFLAGS) $(DEBUG) -c rbtree.c $(LIBS)

clean:
	rm -f *.o buildtree
