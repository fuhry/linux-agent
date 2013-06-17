CC=cc
EXE=linux-agent
LIBS=-ldevmapper
DEBUG=-g

main: cow.o main.c
	mkdir -p bin
	$(CC) $(DEBUG) -Wall -o bin/$(EXE) cow.o main.c $(LIBS)

cow.o: cow.c
	$(CC) $(DEBUG) -Wall -c cow.c

clean:
	rm *.o $(EXE)
