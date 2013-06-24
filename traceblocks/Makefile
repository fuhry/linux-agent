CC=gcc
EXE=traceblocks
DEBUG=-g
CFLAGS=-D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

traceblocks: traceblocks.c
	$(CC) $(CFLAGS) $(DEBUG) -W -Wall -o $(EXE) traceblocks.c $(LIBS)

clean:
	rm *.o $(EXE); true
