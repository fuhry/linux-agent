# Project Setup
include ../settings.mk
CFLAGS =	-std=c99  -Wall -g -Ilibxfs/include -D_GNU_SOURCE
CXXFLAGS =	-std=c++11 -Wall -g -Ilibxfs/include -D_GNU_SOURCE
LDFLAGS =	-std=c++11 -static -Llibxfs -lxfs -lext2fs -lreiserfs -lcom_err -ldal -luuid -lblkid -lpthread -lm
############################################################
EXDEPS=test.cpp Partition.cpp EXTFSPartition.cpp

all: $(EXDEPS)
	cd libxfs; make
	$(CXX) -o $(EXECNAME) $(EXDEPS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	cd libxfs; make clean
	rm -f *.o

cleaner: clean
	cd libxfs; make cleaner
	rm -f $(EXECNAME)

