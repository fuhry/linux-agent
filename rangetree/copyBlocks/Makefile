CC=gcc
DEBUG=-g
CFLAGS=-D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

copyblocks: copyblocks.c rangetree.o rbtree.o copychanges.o
	$(CC) $(CFLAGS) $(DEBUG) -W -Wall -o copyblocks copyblocks.c rangetree.o rbtree.o copychanges.o $(LIBS)

copyblocks.o: copyblocks.c rbtree.o rangetree.o
	$(CC) $(CFLAGS) $(DEBUG) -c -o copyblocks.o copyblocks.c $(LIBS)

rangetree.o: rbtree.o
	$(CC) $(CFLAGS) $(DEBUG) -c -o rangetree.o ../rangetree.c $(LIBS)

copychanges.o: copychanges.c rbtree.o rangetree.o
	$(CC) $(CFLAGS) $(DEBUG) -c -o copychanges.o copychanges.c $(LIBS)

rbtree.o:
	$(CC) $(CFLAGS) $(DEBUG) -c -o rbtree.o ../rbtree.c $(LIBS)

clean:
	rm -f *.o copyblocks
