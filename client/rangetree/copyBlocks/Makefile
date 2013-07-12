CC=gcc
DEBUG=-g
CFLAGS=-D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

copyblocks: copyblocks.c ../rangetree.o ../rbtree.o copychanges.o
	$(CC) $(CFLAGS) $(DEBUG) -W -Wall -o copyblocks copyblocks.c ../rangetree.o ../rbtree.o copychanges.o $(LIBS)

copyblocks.o: copyblocks.c ../rbtree.o ../rangetree.o
	$(CC) $(CFLAGS) $(DEBUG) -c copyblocks.c $(LIBS)

buildtree: ../buildtree.c ../rangetree.o ../rbtree.o
	$(CC) $(CFLAGS) $(DEBUG) -W -Wall -o ../buildtree ../buildtree.c ../rangetree.o ../rbtree.o $(LIBS)

rangetree.o: ../rangetree.c ../rbtree.o
	$(CC) $(CFLAGS) $(DEBUG) -c ../rangetree.c $(LIBS)

copychanges.o: copychanges.c ../rbtree.o ../rangetree.o
	$(CC) $(CFLAGS) $(DEBUG) -c copychanges.c $(LIBS)

rbtree.o: ../rbtree.c
	$(CC) $(CFLAGS) $(DEBUG) -c ../rbtree.c $(LIBS)

clean:
	rm -f *.o buildtree
