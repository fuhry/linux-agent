# Project Setup
include ../../settings.mk
ARNAME =	libbtrfs.a
CFLAGS =	-std=gnu99 -g -D_GNU_SOURCE
ARFLAGS =	rcs

############################################################
DEPS=extent_io.o extent-tree.o print-tree.o crc32c.o inode-map.o utils.o rbtree.o inode-item.o disk-io.o ctree.o file-item.o volumes.o extent-cache.o repair.o root-tree.o radix-tree.o dir-item.o free-space-cache.o raid6.o

all: $(DEPS)
	$(AC) $(ARFLAGS) $(ARNAME) $(DEPS) 

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o

cleaner: clean
	rm -f $(ARNAME)

