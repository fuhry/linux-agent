# Project Setup
include ../settings.mk
CFLAGS =	-std=c99  -Wall -g -Ilibxfs/include -Ilibbtrfs/include -D_GNU_SOURCE
LDFLAGS =	-static -Llibxfs -Llibbtrfs -lxfs -lbtrfs -lext2fs -lreiserfs -lcom_err -ldal -luuid -lblkid -lpthread -lm
ARFLAGS =	rcs
############################################################
ARDEPS=fsparser.o ext.o xfs.o reiser.o btrfs.o
EXDEPS=test.o $(ARDEPS)

all: $(EXDEPS)
	cd libxfs; make
	cd libbtrfs; make
	$(CC) -o $(EXECNAME) $(EXDEPS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

lib:
	$(AC) $(ARFLAGS) $(FSARNAME) $(ARDEPS)

clean:
	cd libxfs; make clean
	cd libbtrfs; make clean
	rm -f *.o

cleaner: clean
	cd libxfs; make cleaner
	cd libbtrfs; make cleaner
	rm -f $(EXECNAME) $(FSARNAME)

