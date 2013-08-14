/**
	File:        xfs.c
	Author:      Charles Roydhouse
	Description: xfs filesystem parsing functions
*/


#include "xfs.h"     /* Local */
#include "tools.h"
#include <stdbool.h> /* Standard */
#include <error.h>   /* GNU */
#include <libxfs.h>  /* Filesystem */

#define XFS_SUPERBLOCK_LOC 0x00

typedef struct ag_header { /*because the xfs library is BAD*/
	xfs_dsb_t *xfs_sb;
	xfs_agf_t *xfs_agf;
	xfs_agi_t *xfs_agi;
	xfs_agfl_t *xfs_agfl;
	char *residue;
	int residue_length;
} ag_header_t;

struct xfs_device_info {
	char *dev;
	int fd;
	xfs_mount_t *mp;
	int block_size;
	int sector_size;
	int first_agbno;
};

int xfs_has_identifier(int fd) {
	uint32_t signature;
	xfs_sb_t super;
	
	/* Seek to superblock */
	if(lseek(fd, XFS_SUPERBLOCK_LOC, SEEK_SET) < 0) {
		return false;
	}
	
	/* Read superblock into struct */
	if(read(fd, &super, sizeof(super)) < 0) {
		return false;
	}
	
	/*
	  Switch byte-order to big endian if needed,
	  xfs's header doesn't do this for us!
	*/
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	signature = letobe32(super.sb_magicnum);
#endif

	return signature == XFS_SB_MAGIC;
}


int xfs_iter_ag(int agno, struct xfs_device_info *devinfo, int (*callback)(int fd, uint64_t length, uint64_t offset)) {
	int rc = 0;
	ag_header_t ag_hdr;
	xfs_daddr_t read_ag_off;
	int read_ag_length;
	void *read_ag_buf = NULL;
	xfs_off_t read_ag_position;
	uint64_t current_block, block_count;
	void *btree_buf_data = NULL;
	int btree_buf_length;
	xfs_off_t btree_buf_position;
	xfs_alloc_ptr_t *ptr = NULL;
	xfs_agblock_t bno;
	xfs_daddr_t ag_end = 0, next_begin = 0, ag_begin = 0, begin = 0, new_begin = 0;
	struct xfs_btree_block* block;
	xfs_off_t pos;
	int length;
	xfs_alloc_rec_t *rec_ptr;
	int w_size = 1 * 1024 * 1024;
	int wblocks = 0;
	uint64_t numblocks = 0;
	int w_length = 0;
	xfs_off_t w_position;
	uint64_t size, sizeb;
	
	read_ag_off = XFS_AG_DADDR(devinfo->mp, agno, XFS_SB_DADDR);
	read_ag_length = devinfo->first_agbno * devinfo->block_size;
	read_ag_position = (xfs_off_t) read_ag_off * (xfs_off_t) BBSIZE;
	read_ag_buf = malloc(read_ag_length);
	
	memset(read_ag_buf, 0, read_ag_length);
	
	if(lseek(devinfo->fd, read_ag_position, SEEK_SET) < 0) {
		error(0, errno, "Error seeking %s (ag position: %ld)", devinfo->dev, read_ag_position);
		goto out;
	}
	
	current_block = (read_ag_position/devinfo->block_size);
	block_count = (read_ag_length/devinfo->block_size);
	
	callback(devinfo->fd, read_ag_length, read_ag_position);
	rc += read_ag_length;
	
	if(read(devinfo->fd, read_ag_buf, read_ag_length) < 0) {
		error(0, errno, "Error reading %s (ag position: %ld)", devinfo->dev, read_ag_position);
		goto out;
	}
	
	ag_hdr.xfs_sb = (xfs_dsb_t*) (read_ag_buf);
	ag_hdr.xfs_agf = (xfs_agf_t*) ((char*) read_ag_buf + devinfo->sector_size);
	ag_hdr.xfs_agi = (xfs_agi_t*) ((char*) read_ag_buf + 2 * devinfo->sector_size);
	ag_hdr.xfs_agfl = (xfs_agfl_t*) ((char*) read_ag_buf + 3 * devinfo->sector_size);
	
	btree_buf_data = malloc(devinfo->block_size);
	memset(btree_buf_data, 0, devinfo->block_size);
	memmove(btree_buf_data, ag_hdr.xfs_agf, devinfo->sector_size);
	ag_hdr.xfs_agf = (xfs_agf_t*) btree_buf_data;
	btree_buf_length = devinfo->block_size;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	bno = betole32(ag_hdr.xfs_agf->agf_roots[XFS_BTNUM_BNOi]);
	ag_end = XFS_AGB_TO_DADDR(devinfo->mp, agno, betole32(ag_hdr.xfs_agf->agf_length) - 1) + devinfo->block_size / BBSIZE;
#else
	bno = ag_hdr.xfs_agf->agf_roots[XFS_BTNUM_BNOi];
	ag_end = XFS_AGB_TO_DADDR(devinfo->mp, agno, ag_hdr.xfs_agf->agf_length - 1) + devinfo->block_size / BBSIZE;
#endif

	for(uint32_t current_level = 1;; ++current_level) {
		uint16_t bb_level;
		btree_buf_position = (xfs_off_t) XFS_AGB_TO_DADDR(devinfo->mp, agno, bno) << BBSHIFT;
		btree_buf_length = devinfo->block_size;
		
		if(lseek(devinfo->fd, btree_buf_position, SEEK_SET) < 0) {
			error(0, errno, "Error seeking %s (btree position: %ld)", devinfo->dev, btree_buf_position);
			goto out;
		}
		
		current_block = (btree_buf_position/devinfo->block_size);
		block_count = (btree_buf_length/devinfo->block_size);	
		callback(devinfo->fd, btree_buf_length, btree_buf_position);	
		rc += btree_buf_length;
			
		if(read(devinfo->fd, btree_buf_data, btree_buf_length) < 0) {
			error(0, errno, "Error reading %s (btree position: %ld)", devinfo->dev, btree_buf_position);
			goto out;
		}
		block = (struct xfs_btree_block*) btree_buf_data;
		bb_level = block->bb_level;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		bb_level = betole16(block->bb_level);
#endif
		if(bb_level == 0) break;		
		ptr = XFS_ALLOC_PTR_ADDR(devinfo->mp, block, 1, devinfo->mp->m_alloc_mxr[1]);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		bno = betole32(ptr[0]);
#else
		bno = ptr[0];
#endif
	}
	
	pos = read_ag_position >> BBSHIFT;
	length = read_ag_length >> BBSHIFT;
	next_begin = pos + length;
	ag_begin = next_begin;
	
	for(;;) {
		rec_ptr = XFS_ALLOC_REC_ADDR(devinfo->mp, block, 1);
		uint16_t bb_numrecs = block->bb_numrecs;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		bb_numrecs = betole16(bb_numrecs);
#endif
		for(int i = 0; i < bb_numrecs; ++i, ++rec_ptr) {
			begin = next_begin;
			if(begin < ag_begin) {
				begin = ag_begin;
			}
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			sizeb = XFS_AGB_TO_DADDR(devinfo->mp, agno, betole32(rec_ptr->ar_startblock)) - begin;
#else
			sizeb = XFS_AGB_TO_DADDR(devinfo->mp, agno, rec_ptr->ar_startblock) - begin;
#endif
			size = roundup(sizeb << BBSHIFT, devinfo->sector_size);		
		
			w_position = (xfs_off_t) begin << BBSHIFT;
			while(size > 0) {
				if(size > w_size) {
					w_length = w_size;
					size -= w_size;
					sizeb -= wblocks;
					numblocks += wblocks;
				} else {
					w_length = size;
					numblocks += sizeb;
					size = 0;
				}
				if(lseek(devinfo->fd, w_position, SEEK_SET) < 0) {
					error(0, errno, "Error seeking %s (w position: %ld)", devinfo->dev, w_position);
					goto out;
				}
				current_block = (w_position/devinfo->block_size);
				block_count = (w_length/devinfo->block_size);		
				callback(devinfo->fd, w_length, w_position);
				rc += w_length;
		
				if(lseek(devinfo->fd, w_length, SEEK_CUR) < 0) {
					error(0, errno, "Error seeking %s (w length: %d)", devinfo->dev, w_length);
					goto out;
				}
				w_position += w_length;
			}
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			new_begin = XFS_AGB_TO_DADDR(devinfo->mp, agno, betole32(rec_ptr->ar_startblock) + betole32(rec_ptr->ar_blockcount));
#else
			new_begin = XFS_AGB_TO_DADDR(devinfo->mp, agno, rec_ptr->ar_startblock + rec_ptr->ar_blockcount);
#endif
			next_begin = rounddown(new_begin, devinfo->sector_size >> BBSHIFT);
		}

	
		uint32_t bb_rightsib = block->bb_u.s.bb_rightsib;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		bb_rightsib = betole32(bb_rightsib);
#endif
		if(bb_rightsib == NULLAGBLOCK) break;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		btree_buf_position = pos = (xfs_off_t)XFS_AGB_TO_DADDR(devinfo->mp, agno, betole32(block->bb_u.s.bb_rightsib)) << BBSHIFT;
#else
		btree_buf_position = pos = (xfs_off_t)XFS_AGB_TO_DADDR(devinfo->mp, agno, block->bb_u.s.bb_rightsib) << BBSHIFT;
#endif

		btree_buf_length = devinfo->block_size;
		if(lseek(devinfo->fd, btree_buf_position, SEEK_SET) < 0) {
			error(0, errno, "Error seeking %s (btree pos: %ld)", devinfo->dev, btree_buf_position);
			goto out;
		}
		current_block = (btree_buf_position/devinfo->block_size);
		block_count = (btree_buf_length/devinfo->block_size);
		callback(devinfo->fd, btree_buf_length, btree_buf_position);
		rc += btree_buf_length;
		
		if(read(devinfo->fd, btree_buf_data, btree_buf_length) < 0) {
			error(0, errno, "Error reading %s (btree pos: %ld)", devinfo->dev, btree_buf_position);
			goto out;
		}
		block = (struct xfs_btree_block*) btree_buf_data;
	
		if(next_begin < ag_end) {
			begin = next_begin;
			sizeb = ag_end - begin;
			size = roundup(sizeb << BBSHIFT, devinfo->sector_size);
			w_position = (xfs_off_t) begin << BBSHIFT;

			while (size > 0) {
				if (size > w_size) {
					w_length = w_size;
					size -= w_size;
					sizeb -= wblocks;
					numblocks += wblocks;
				} else {
					w_length = size;
					numblocks += sizeb;
					size = 0;
				}

				if(lseek(devinfo->fd, w_position, SEEK_SET) < 0) {
					error(0, errno, "Error seeking %s (w pos: %ld)", devinfo->dev, w_position);
					goto out;
				}
				current_block = (w_position/devinfo->block_size);
				block_count = (w_length/devinfo->block_size);
				callback(devinfo->fd, w_length, w_position);
				rc += w_length;
			
				if(lseek(devinfo->fd, w_length, SEEK_CUR) < 0) {
					error(0, errno, "Error seeking %s (w length: %d)", devinfo->dev, w_length);
					goto out;
				}
				w_position += w_length;	
			}
		}

		int log_length = 1 * 1024 * 1024;
		int logstart = XFS_FSB_TO_DADDR(devinfo->mp, devinfo->mp->m_sb.sb_logstart) << BBSHIFT;
		int logstart_pos = rounddown(logstart, (xfs_off_t) log_length);
		if(logstart % log_length) { 
			if(lseek(devinfo->fd, logstart_pos, SEEK_SET) < 0) {
				error(0, errno, "Error seeking %s (logstart pos: %d)", devinfo->dev, logstart_pos);
				goto out;
			}
			current_block = (logstart_pos/devinfo->block_size);
			block_count = (log_length/devinfo->block_size);
			callback(devinfo->fd, log_length, logstart_pos);
			rc += log_length;
		
			if(lseek(devinfo->fd, log_length, SEEK_CUR) < 0) {
				error(0, errno, "Error seeking %s (loglength: %d)", devinfo->dev, log_length);
				goto out;
			}
		}
	
		int logend = XFS_FSB_TO_DADDR(devinfo->mp, devinfo->mp->m_sb.sb_logstart) << BBSHIFT;
		logend += XFS_FSB_TO_B(devinfo->mp, devinfo->mp->m_sb.sb_logblocks);
		int logend_pos = rounddown(logend, (xfs_off_t) log_length);

		if(logend % log_length) { 
			if(lseek(devinfo->fd, logend_pos, SEEK_SET) < 0) {
				error(0, errno, "Error seeking %s (logstart pos: %d)", devinfo->dev, logstart_pos);
				goto out;
			}
			current_block = (logend_pos/devinfo->block_size);
			block_count = (log_length/devinfo->block_size);
			callback(devinfo->fd, log_length, logend_pos);
			rc += log_length;
		
			if(lseek(devinfo->fd, log_length, SEEK_CUR) < 0) {
				error(0, errno, "Error seeking %s (loglength: %d)", devinfo->dev, log_length);
				goto out;
			}
		}
	}
out:
	if(read_ag_buf) free(read_ag_buf);
	if(btree_buf_data) free(btree_buf_data);
	if(ptr) free(ptr);
	return rc;
}


int xfs_iter_blocks(const char *dev, int (*callback)(int fd, uint64_t length, uint64_t offset)) {
	int rc = 0;
	libxfs_init_t xargs;
	xfs_mount_t *mp;
	xfs_mount_t mbuf;
	xfs_sb_t *super;
	xfs_buf_t *sbp;
	int block_size;
	int sector_size;
	int first_residue;
	xfs_agblock_t first_agbno;
	xfs_agnumber_t ag_count;
		
	int fd = open(dev, O_RDONLY);
	if(fd < 0) { 
		error(0, errno, "Error opening %s", dev);
		goto out;
	}
	
	memset(&xargs, 0, sizeof(xargs));
	xargs.isdirect = LIBXFS_DIRECT;
	xargs.isreadonly = LIBXFS_ISREADONLY;
	xargs.volname = (char*) dev;
	if(!libxfs_init(&xargs)) {
		error(0, errno, "Error initializing xfs partition %s", dev);
		goto out;
	}
	
	//    libxfs_readbuf(dev, daddr, len, flags, ops)
	sbp = libxfs_readbuf(xargs.ddev, XFS_SB_DADDR, 1, 0);
	
	memset(&mbuf, 0, sizeof(xfs_mount_t));
	super = &mbuf.m_sb;
	libxfs_sb_from_disk(super, XFS_BUF_TO_SBP(sbp));
	mp = libxfs_mount(&mbuf, super, xargs.ddev, xargs.logdev, xargs.rtdev, 1);
	
	if(!mp || mp->m_sb.sb_inprogress || !mp->m_sb.sb_logstart || mp->m_sb.sb_rextents) {
		error(0, errno, "Failed to initialize %s", dev);
		goto out;
	}
	
	block_size = mp->m_sb.sb_blocksize;
	sector_size = mp->m_sb.sb_sectsize;
	
	if(block_size > sector_size)  {
		int leftover = ((XFS_AGFL_DADDR(mp) + 1) * sector_size) % block_size;
		first_residue = (leftover == 0) ? 0 :  block_size - leftover;
	} else if(block_size == sector_size) {
		first_residue = 0;
	} else {
		error(0, errno, "Fatal: %s block size < sector size", dev);
		goto out;
	}
	 
	first_agbno = (((XFS_AGFL_DADDR(mp) + 1) * sector_size) + first_residue) / block_size;
	ag_count = mp->m_sb.sb_agcount;
	
	struct xfs_device_info devinfo;
	devinfo.dev = (char*) dev;
	devinfo.fd = fd;
	devinfo.mp = mp;
	devinfo.block_size = block_size;
	devinfo.sector_size = sector_size;
	devinfo.first_agbno = first_agbno;
	
	for(xfs_agnumber_t agno = 0; agno < ag_count; ++agno) {
		rc += xfs_iter_ag(agno, &devinfo, callback);
	}

out:
	libxfs_device_close(xargs.ddev);  
	if(fd >= 0) {
		close(fd);
	}
	return rc;
}
