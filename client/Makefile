CC=gcc
DEBUG=-g
CFLAGS=-D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
LIBS=-lrt -lpthread

tracetester: tracetester.c trace.c rangetree.o rbtree.o
	$(CC) $(CFLAGS) $(DEBUG) -W -Wall -o tracetester tracetester.c trace.c rangetree/rangetree.o rangetree/rbtree.o $(LIBS) 

dattod: dattod.c
	$(CC) $(CFLAGS) $(DEBUG) -W -Wall -o dattod dattod.c $(LIBS)

rangetree.o:
	$(CC) $(CFLAGS) $(DEBUG) -W -Wall -c -o rangetree/rangetree.o ./rangetree/rangetree.c $(LIBS)

rbtree.o:
	$(CC) $(CFLAGS) $(DEBUG) -W -Wall -c -o rangetree/rbtree.o ./rangetree/rbtree.c $(LIBS)

clean:
	rm -f rangetree/rangetree.o rangetree/rbtree.o dattod tracetester
