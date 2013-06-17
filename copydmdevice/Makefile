CC=gcc
EXE=copydmdevice
LIBS=-ldevmapper -lext2fs
DEBUG=-g
CFLAGS=-D_FILE_OFFSET_BITS=64

main: cow.o extfs_blockcopy.o main.c
	$(CC) $(CFLAGS) $(DEBUG) -Wall -o $(EXE) extfs_blockcopy.o cow.o main.c $(LIBS)

cow.o: cow.c
	$(CC) $(CFLAGS) $(DEBUG) -Wall -c cow.c

extfs_blockcopy.o: extfs_blockcopy.c
	$(CC) $(CFLAGS) $(DEBUG) -Wall -c extfs_blockcopy.c

clean:
	rm *.o $(EXE)
