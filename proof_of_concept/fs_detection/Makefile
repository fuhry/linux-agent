CC=gcc
EXE=detect_fs
LIBS=-lblkid
DEBUG=-g
CFLAGS=-D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

detect_fs: detect_fs.c
	$(CC) $(CFLAGS) $(DEBUG) -W -Wall -o $(EXE) detect_fs.c $(LIBS)

clean:
	rm -f *.o $(EXE)
