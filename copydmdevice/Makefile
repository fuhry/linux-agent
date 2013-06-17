CC=cc
EXE=copydmdevice
LIBS=-ldevmapper
DEBUG=-g

main: cow.o main.c
	$(CC) $(DEBUG) -Wall -o $(EXE) cow.o main.c $(LIBS)

cow.o: cow.c
	$(CC) $(DEBUG) -Wall -c cow.c

clean:
	rm *.o $(EXE)
