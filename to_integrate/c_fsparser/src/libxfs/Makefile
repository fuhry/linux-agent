# Project Setup
include ../../settings.mk
ARNAME =	libxfs.a
CFLAGS =	-std=gnu99 -g -D_GNU_SOURCE
ARFLAGS =	rcs

############################################################
DEPS=xfs_rtalloc.o kmem.o init.o xfs_dir2_data.o logitem.o xfs_alloc.o cache.o xfs_btree.o xfs_attr_leaf.o trans.o xfs_dir2_node.o util.o xfs_da_btree.o xfs_dir2_leaf.o xfs_ialloc_btree.o xfs_dir2_sf.o xfs_dir2.o xfs_attr.o linux.o crc32.o xfs_mount.o rdwr.o xfs_trans.o xfs_inode.o xfs_dir2_block.o xfs_bmap.o xfs_ialloc.o xfs_alloc_btree.o radix-tree.o xfs_bmap_btree.o 

all: $(DEPS)
	$(AC) $(ARFLAGS) $(ARNAME) $(DEPS) 

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o

cleaner: clean
	rm -f $(ARNAME)

