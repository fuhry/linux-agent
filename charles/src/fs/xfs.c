/**
	File:        xfs.c
	Author:      Charles Roydhouse
	Description: xfs filesystem parsing functions
*/

#include "xfs.h"
#include "fs.h"
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <xfs/libxfs.h>
#include <xfs/xfs_types.h>

typedef struct ag_header { /*because the xfs library is BAD*/
	xfs_dsb_t *xfs_sb;
	xfs_agf_t *xfs_agf;
	xfs_agi_t *xfs_agi;
	xfs_agfl_t *xfs_agfl;
	char *residue;
	int residue_length;
} ag_header_t;


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
	
	/* Switch byte-order to big endian if needed */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	signature = letobe32(super.sb_magicnum);
#endif

	return signature == XFS_SIGNATURE;
}


int xfs_iter_blocks(const char *dev, int (*callback)(int fd, uint64_t length, uint64_t offset)) {
	int rc = 0;
	int block_size;
	int sector_size;
	int first_residue;
	libxfs_init_t xargs;
	xfs_mount_t *mp;
	xfs_mount_t mbuf;
	xfs_sb_t *super;
	xfs_buf_t *sbp;
	
	ag_header_t ag_hdr;
	xfs_agblock_t first_agbno;
	xfs_agnumber_t agno;
  xfs_agnumber_t ag_count;
	xfs_daddr_t read_ag_off;
	int read_ag_length;
	void *read_ag_buf = NULL;
	xfs_off_t	read_ag_position;
	uint64_t current_block, block_count;
	void *btree_buf_data = NULL;
  int btree_buf_length;
	xfs_off_t	btree_buf_position;
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
	for(agno = 0; agno < ag_count; ++agno)  {
		read_ag_off = XFS_AG_DADDR(mp, agno, XFS_SB_DADDR);
		read_ag_length = first_agbno * block_size;
		read_ag_position = (xfs_off_t) read_ag_off * (xfs_off_t) BBSIZE;
		read_ag_buf = malloc(read_ag_length);
		
		memset(read_ag_buf, 0, read_ag_length);
		
		if(lseek(fd, read_ag_position, SEEK_SET) < 0) {
			error(0, errno, "Error seeking %s (ag position: %ld)", dev, read_ag_position);
			goto out;
		}
		
		current_block = (read_ag_position/block_size);
		block_count = (read_ag_length/block_size);
		
		callback(fd, read_ag_length, read_ag_position);
		rc += read_ag_length;
		
		if(read(fd, read_ag_buf, read_ag_length) < 0) {
			error(0, errno, "Error reading %s (ag position: %ld)", dev, read_ag_position);
			goto out;
		}
		
		ag_hdr.xfs_sb = (xfs_dsb_t*) (read_ag_buf);
		ag_hdr.xfs_agf = (xfs_agf_t*) ((char*) read_ag_buf + sector_size);
		ag_hdr.xfs_agi = (xfs_agi_t*) ((char*) read_ag_buf + 2 * sector_size);
		ag_hdr.xfs_agfl = (xfs_agfl_t*) ((char*) read_ag_buf + 3 * sector_size);
		
		btree_buf_data = malloc(block_size);
		memset(btree_buf_data, 0, block_size);
		memmove(btree_buf_data, ag_hdr.xfs_agf, sector_size);
		ag_hdr.xfs_agf = (xfs_agf_t*) btree_buf_data;
		btree_buf_length = block_size;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		bno = betole32(ag_hdr.xfs_agf->agf_roots[XFS_BTNUM_BNOi]);
		ag_end = XFS_AGB_TO_DADDR(mp, agno, betole32(ag_hdr.xfs_agf->agf_length) - 1) + block_size / BBSIZE;
#else
		bno = ag_hdr.xfs_agf->agf_roots[XFS_BTNUM_BNOi];
		ag_end = XFS_AGB_TO_DADDR(mp, agno, ag_hdr.xfs_agf->agf_length - 1) + block_size / BBSIZE;
#endif

		for(uint32_t current_level = 1;; ++current_level) {
			uint16_t bb_level;
			btree_buf_position = (xfs_off_t) XFS_AGB_TO_DADDR(mp, agno, bno) << BBSHIFT;
			btree_buf_length = block_size;
			
			if(lseek(fd, btree_buf_position, SEEK_SET) < 0) {
				error(0, errno, "Error seeking %s (btree position: %ld)", dev, btree_buf_position);
				goto out;
			}
			
			current_block = (btree_buf_position/block_size);
			block_count = (btree_buf_length/block_size);	
			callback(fd, btree_buf_length, btree_buf_position);		
			rc += btree_buf_length;
					
			if(read(fd, btree_buf_data, btree_buf_length) < 0) {
				error(0, errno, "Error reading %s (btree position: %ld)", dev, btree_buf_position);
				goto out;
			}
			block = (struct xfs_btree_block*) btree_buf_data;
			bb_level = block->bb_level;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			bb_level = betole16(block->bb_level);
#endif
			if(bb_level == 0) break;
			
			ptr = XFS_ALLOC_PTR_ADDR(mp, block, 1, mp->m_alloc_mxr[1]);


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
			rec_ptr = XFS_ALLOC_REC_ADDR(mp, block, 1);
			uint16_t bb_numrecs = block->bb_numrecs;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			bb_numrecs = betole16(bb_numrecs);
#endif
			for(int i = 0; i < bb_numrecs; ++i, ++rec_ptr) {
				begin = next_begin;
				if(begin < ag_begin)
					begin = ag_begin;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
				sizeb = XFS_AGB_TO_DADDR(mp, agno, betole32(rec_ptr->ar_startblock)) - begin;
#else
				sizeb = XFS_AGB_TO_DADDR(mp, agno, rec_ptr->ar_startblock) - begin;
#endif
				size = roundup(sizeb << BBSHIFT, sector_size);
			
			
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
					if(lseek(fd, w_position, SEEK_SET) < 0) {
						error(0, errno, "Error seeking %s (w position: %ld)", dev, w_position);
						goto out;
					}
					current_block = (w_position/block_size);
					block_count = (w_length/block_size);				
					callback(fd, w_length, w_position);
					rc += w_length;
			
					if(lseek(fd, w_length, SEEK_CUR) < 0) {
						error(0, errno, "Error seeking %s (w length: %d)", dev, w_length);
						goto out;
					}
					w_position += w_length;
				}
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
				new_begin = XFS_AGB_TO_DADDR(mp, agno, betole32(rec_ptr->ar_startblock) + betole32(rec_ptr->ar_blockcount));
#else
				new_begin = XFS_AGB_TO_DADDR(mp, agno, rec_ptr->ar_startblock + rec_ptr->ar_blockcount);
#endif
				next_begin = rounddown(new_begin, sector_size >> BBSHIFT);
		}

		
			uint32_t bb_rightsib = block->bb_u.s.bb_rightsib;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			bb_rightsib = betole32(bb_rightsib);
#endif
			if(bb_rightsib == NULLAGBLOCK) break;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			btree_buf_position = pos = (xfs_off_t)XFS_AGB_TO_DADDR(mp, agno, betole32(block->bb_u.s.bb_rightsib)) << BBSHIFT;
#else
			btree_buf_position = pos = (xfs_off_t)XFS_AGB_TO_DADDR(mp, agno, block->bb_u.s.bb_rightsib) << BBSHIFT;
#endif

			btree_buf_length = block_size;
			if(lseek(fd, btree_buf_position, SEEK_SET) < 0) {
					error(0, errno, "Error seeking %s (btree pos: %ld)", dev, btree_buf_position);
					goto out;
			}
			current_block = (btree_buf_position/block_size);
			block_count = (btree_buf_length/block_size);
			callback(fd, btree_buf_length, btree_buf_position);
			rc += btree_buf_length;
			
			if(read(fd, btree_buf_data, btree_buf_length) < 0) {
					error(0, errno, "Error reading %s (btree pos: %ld)", dev, btree_buf_position);
					goto out;
			}
			block = (struct xfs_btree_block*) btree_buf_data;
	
			if(next_begin < ag_end) {
				begin = next_begin;
				sizeb = ag_end - begin;
				size = roundup(sizeb << BBSHIFT, sector_size);
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

					if(lseek(fd, w_position, SEEK_SET) < 0) {
						error(0, errno, "Error seeking %s (w pos: %ld)", dev, w_position);
						goto out;
					}
					current_block = (w_position/block_size);
					block_count = (w_length/block_size);
					callback(fd, w_length, w_position);
					rc += w_length;
					
					if(lseek(fd, w_length, SEEK_CUR) < 0) {
						error(0, errno, "Error seeking %s (w length: %d)", dev, w_length);
						goto out;
					}
					w_position += w_length;		
				}
			}

			int log_length = 1 * 1024 * 1024;
			int logstart = XFS_FSB_TO_DADDR(mp, mp->m_sb.sb_logstart) << BBSHIFT;
			int logstart_pos = rounddown(logstart, (xfs_off_t) log_length);
			if(logstart % log_length) { 
				if(lseek(fd, logstart_pos, SEEK_SET) < 0) {
					error(0, errno, "Error seeking %s (logstart pos: %d)", dev, logstart_pos);
					goto out;
				}
				current_block = (logstart_pos/block_size);
				block_count = (log_length/block_size);
				callback(fd, log_length, logstart_pos);
				rc += log_length;
			
				if(lseek(fd, log_length, SEEK_CUR) < 0) {
					error(0, errno, "Error seeking %s (loglength: %d)", dev, log_length);
					goto out;
				}
			}
	
			int logend = XFS_FSB_TO_DADDR(mp, mp->m_sb.sb_logstart) << BBSHIFT;
			logend += XFS_FSB_TO_B(mp, mp->m_sb.sb_logblocks);
			int logend_pos = rounddown(logend, (xfs_off_t)log_length);

			if(logend % log_length) { 
				if(lseek(fd, logend_pos, SEEK_SET) < 0) {
					error(0, errno, "Error seeking %s (logstart pos: %d)", dev, logstart_pos);
					goto out;
				}
				current_block = (logend_pos/block_size);
				block_count = (log_length/block_size);
				callback(fd, log_length, logend_pos);
				rc += log_length;
				
				if(lseek(fd, log_length, SEEK_CUR) < 0) {
					error(0, errno, "Error seeking %s (loglength: %d)", dev, log_length);
					goto out;
				}
			}
		}
	}

out:
	libxfs_device_close(xargs.ddev);
	if(read_ag_buf) free(read_ag_buf);
  if(btree_buf_data) free(btree_buf_data);
  if(ptr) free(ptr);
  
	if(!(fd < 0)) {
		close(fd);
	}
	return rc;
}
