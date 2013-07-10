CC=gcc
DEBUG=-g
CFLAGS=-D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

buildtree: dattod.c
	$(CC) $(CFLAGS) $(DEBUG) -lrt -W -Wall -o dattod dattod.c

clean:
	rm -f *.o dattod
