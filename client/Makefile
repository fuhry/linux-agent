CC=gcc
DEBUG=-g
CFLAGS=-D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
LIBS=-lrt -lpthread

dattod: dattod.c
	$(CC) $(CFLAGS) $(DEBUG) -W -Wall -o dattod dattod.c $(LIBS)

clean:
	rm -f *.o dattod
