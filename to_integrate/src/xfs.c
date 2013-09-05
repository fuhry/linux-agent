/**
	File:        xfs.c
	Author:      Charles Roydhouse
	Description: xfs filesystem parsing functions
*/

#include "xfs.h"
#include "tools.h"

#include <error.h>
#include <libxfs.h>
#include <stdbool.h>

#define XFS_SUPERBLOCK_LOC 0x00
#define MAX_WRITE_BLOCK_LENGTH 0x100000
#define JOURNAL_LOG_LENGTH 0x100000

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


/** Parses blocks with in an allocation group (records, btree, journal, etc) */
int xfs_iter_allocation_group_blocks(int agno, xfs_daddr_t ag_begin,
	xfs_daddr_t ag_end, struct xfs_btree_block* initial_block,
	struct xfs_device_info *devinfo, int (*callback)(int fd, uint64_t length,
	uint64_t offset)) {
	
	int rc = 0;
	
	void *btree_buf_data = malloc(devinfo->block_size);
	xfs_off_t btree_buf_position;
	struct xfs_btree_block *block = initial_block;
	
	xfs_alloc_rec_t *record_ptr;
	xfs_daddr_t begin = ag_begin;
		
	int write_block_length = 0;
	xfs_off_t write_position;
	uint64_t to_read;
	uint64_t sizeb;

	while(true) {
		record_ptr = XFS_ALLOC_REC_ADDR(devinfo->mp, block, 1);
		
		/* Handle records */
		uint16_t bb_numrecs = block->bb_numrecs;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		bb_numrecs = betole16(bb_numrecs);
#endif
		for(int i = 0; i < bb_numrecs; ++i, ++record_ptr) {
			
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			sizeb = XFS_AGB_TO_DADDR(devinfo->mp, agno,
				betole32(record_ptr->ar_startblock)) - begin;
#else
			sizeb = XFS_AGB_TO_DADDR(devinfo->mp, agno, record_ptr->ar_startblock)
				- begin;
#endif
			to_read = roundup(sizeb << BBSHIFT, devinfo->sector_size);

			write_position = (xfs_off_t) begin << BBSHIFT;
			while (to_read > 0) {
				if(to_read > MAX_WRITE_BLOCK_LENGTH) {
					write_block_length = MAX_WRITE_BLOCK_LENGTH;
					to_read -= MAX_WRITE_BLOCK_LENGTH;
				} else {
					write_block_length = to_read;
					to_read = 0;
				}
				
				if(lseek(devinfo->fd, write_position, SEEK_SET) < 0) {
					error(0, errno, "Error seeking %s (w position: %ld)", devinfo->dev,
						write_position);
					goto out;
				}

				callback(devinfo->fd, write_block_length, write_position);
				rc += write_block_length;

				if(lseek(devinfo->fd, write_block_length, SEEK_CUR) < 0) {
					error(0, errno, "Error seeking %s (w length: %d)", devinfo->dev,
						write_block_length);
					goto out;
				}
				write_position += write_block_length;
			}
		}

		/* Handle btree */
		uint32_t bb_rightsib = block->bb_u.s.bb_rightsib;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		bb_rightsib = betole32(bb_rightsib);
#endif
		if(bb_rightsib == NULLAGBLOCK) break;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		btree_buf_position = (xfs_off_t)XFS_AGB_TO_DADDR(devinfo->mp, agno,
			betole32(block->bb_u.s.bb_rightsib)) << BBSHIFT;
#else
		btree_buf_position = (xfs_off_t)XFS_AGB_TO_DADDR(devinfo->mp, agno,
			block->bb_u.s.bb_rightsib) << BBSHIFT;
#endif

		if(lseek(devinfo->fd, btree_buf_position, SEEK_SET) < 0) {
			error(0, errno, "Error seeking %s (btree pos: %ld)", devinfo->dev,
				btree_buf_position);
			goto out;
		}

		callback(devinfo->fd, devinfo->block_size, btree_buf_position);
		rc += devinfo->block_size;

		if(read(devinfo->fd, btree_buf_data, devinfo->block_size) < 0) {
			error(0, errno, "Error reading %s (btree pos: %ld)", devinfo->dev,
				btree_buf_position);
			goto out;
		}
		block = (struct xfs_btree_block*) btree_buf_data;

		if(begin < ag_end) {
			sizeb = ag_end - begin;
			to_read = roundup(sizeb << BBSHIFT, devinfo->sector_size);
			write_position = (xfs_off_t) begin << BBSHIFT;

			while (to_read > 0) {
				if (to_read > MAX_WRITE_BLOCK_LENGTH) {
					write_block_length = MAX_WRITE_BLOCK_LENGTH;
					to_read -= MAX_WRITE_BLOCK_LENGTH;
				} else {
					write_block_length = to_read;
					to_read = 0;
				}

				if(lseek(devinfo->fd, write_position, SEEK_SET) < 0) {
					error(0, errno, "Error seeking %s (w pos: %ld)", devinfo->dev,
						write_position);
					goto out;
				}

				callback(devinfo->fd, write_block_length, write_position);
				rc += write_block_length;

				if(lseek(devinfo->fd, write_block_length, SEEK_CUR) < 0) {
					error(0, errno, "Error seeking %s (w length: %d)", devinfo->dev,
						write_block_length);
					goto out;
				}
				write_position += write_block_length;
			}
		}

		/* Handle journaling stuff */
		int journal_log_start = XFS_FSB_TO_DADDR(devinfo->mp,
			devinfo->mp->m_sb.sb_logstart) << BBSHIFT;
			
		int journal_log_start_pos = rounddown(journal_log_start,
			(xfs_off_t) JOURNAL_LOG_LENGTH);
			
		if(journal_log_start % JOURNAL_LOG_LENGTH) {
			if(lseek(devinfo->fd, journal_log_start_pos, SEEK_SET) < 0) {
				error(0, errno, "Error seeking %s (journal_log_start_pos: %d)",
					devinfo->dev, journal_log_start_pos);
				goto out;
			}

			callback(devinfo->fd, JOURNAL_LOG_LENGTH, journal_log_start_pos);
			rc += JOURNAL_LOG_LENGTH;

			if(lseek(devinfo->fd, JOURNAL_LOG_LENGTH, SEEK_CUR) < 0) {
				error(0, errno, "Error seeking %s (loglength: %d)", devinfo->dev,
					JOURNAL_LOG_LENGTH);
				goto out;
			}
		}

		int journal_log_end = (XFS_FSB_TO_DADDR(devinfo->mp,
			devinfo->mp->m_sb.sb_logstart) << BBSHIFT) +
			XFS_FSB_TO_B(devinfo->mp, devinfo->mp->m_sb.sb_logblocks);
			
		int journal_log_end_pos = rounddown(journal_log_end,
			(xfs_off_t) JOURNAL_LOG_LENGTH);
			
		if(journal_log_end % JOURNAL_LOG_LENGTH) { 
			if(lseek(devinfo->fd, journal_log_end_pos, SEEK_SET) < 0) {
				error(0, errno, "Error seeking %s (journal_log_start_pos: %d)",
					devinfo->dev, journal_log_start_pos);
				goto out;
			}

			callback(devinfo->fd, JOURNAL_LOG_LENGTH, journal_log_end_pos);
			rc += JOURNAL_LOG_LENGTH;

			if(lseek(devinfo->fd, JOURNAL_LOG_LENGTH, SEEK_CUR) < 0) {
				error(0, errno, "Error seeking %s (loglength: %d)", devinfo->dev,
					JOURNAL_LOG_LENGTH);
				goto out;
			}
		}
	}
out:
	if(btree_buf_data) free(btree_buf_data);
	return rc;
}


/** Parse a given allocation group then call function to read the contents */
int xfs_iter_allocation_group(int agno, struct xfs_device_info *devinfo,
	int (*callback)(int fd, uint64_t length, uint64_t offset)) {
	int rc = 0;

	ag_header_t ag_hdr;
	xfs_daddr_t read_ag_off;
	int read_ag_length;
	void *read_ag_buf = NULL;
	xfs_off_t read_ag_position;

	void *btree_buf_data = NULL;
	xfs_off_t btree_buf_position;

	xfs_agblock_t bno;
	struct xfs_btree_block *block;
	
	xfs_daddr_t ag_begin = 0;
	xfs_daddr_t ag_end = 0;
	xfs_alloc_ptr_t *ptr = NULL;
	
	read_ag_off = XFS_AG_DADDR(devinfo->mp, agno, XFS_SB_DADDR);
	read_ag_length = devinfo->first_agbno * devinfo->block_size;
	read_ag_position = (xfs_off_t) read_ag_off * (xfs_off_t) BBSIZE;
	read_ag_buf = malloc(read_ag_length);

	memset(read_ag_buf, 0, read_ag_length);

	if(lseek(devinfo->fd, read_ag_position, SEEK_SET) < 0) {
		error(0, errno, "Error seeking %s (ag position: %ld)",
			devinfo->dev, read_ag_position);
		goto out;
	}

	callback(devinfo->fd, read_ag_length, read_ag_position);
	rc += read_ag_length;

	if(read(devinfo->fd, read_ag_buf, read_ag_length) < 0) {
		error(0, errno, "Error reading %s (ag position: %ld)",
			devinfo->dev, read_ag_position);
		goto out;
	}

	ag_hdr.xfs_sb = (xfs_dsb_t*) (read_ag_buf);
	ag_hdr.xfs_agf = (xfs_agf_t*) ((char*) read_ag_buf +
		devinfo->sector_size);
		
	ag_hdr.xfs_agi = (xfs_agi_t*) ((char*) read_ag_buf +
		2 * devinfo->sector_size);
		
	ag_hdr.xfs_agfl = (xfs_agfl_t*) ((char*) read_ag_buf +
		3 * devinfo->sector_size);

	btree_buf_data = malloc(devinfo->block_size);
	memset(btree_buf_data, 0, devinfo->block_size);
	memmove(btree_buf_data, ag_hdr.xfs_agf, devinfo->sector_size);
	ag_hdr.xfs_agf = (xfs_agf_t*) btree_buf_data;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	bno = betole32(ag_hdr.xfs_agf->agf_roots[XFS_BTNUM_BNOi]);
	ag_end = XFS_AGB_TO_DADDR(devinfo->mp, agno, 
		betole32(ag_hdr.xfs_agf->agf_length) - 1) + devinfo->block_size / BBSIZE;
#else
	bno = ag_hdr.xfs_agf->agf_roots[XFS_BTNUM_BNOi];
	ag_end = XFS_AGB_TO_DADDR(devinfo->mp, agno,
		ag_hdr.xfs_agf->agf_length - 1) + devinfo->block_size / BBSIZE;
#endif

	for(uint32_t current_level = 1;; ++current_level) {
		uint16_t bb_level;
		btree_buf_position = (xfs_off_t) XFS_AGB_TO_DADDR(devinfo->mp, agno, bno)
			<< BBSHIFT;

		if(lseek(devinfo->fd, btree_buf_position, SEEK_SET) < 0) {
			error(0, errno, "Error seeking %s (btree position: %ld)", devinfo->dev,
				btree_buf_position);
			goto out;
		}

		callback(devinfo->fd, devinfo->block_size, btree_buf_position);
		rc += devinfo->block_size;

		if(read(devinfo->fd, btree_buf_data, devinfo->block_size) < 0) {
			error(0, errno, "Error reading %s (btree position: %ld)", devinfo->dev,
				btree_buf_position);
			goto out;
		}
		block = (struct xfs_btree_block*) btree_buf_data;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		bb_level = betole16(block->bb_level);
#else
		bb_level = block->bb_level;
#endif

		if(bb_level == 0) break;
		ptr = XFS_ALLOC_PTR_ADDR(devinfo->mp, block, 1,
			devinfo->mp->m_alloc_mxr[1]);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		bno = betole32(ptr[0]);
#else
		bno = ptr[0];
#endif
	}

	ag_begin = (read_ag_position >> BBSHIFT) + (read_ag_length >> BBSHIFT);
	
	rc += xfs_iter_allocation_group_blocks(agno, ag_begin, ag_end, block,
		devinfo, callback);

out:
	if(read_ag_buf) free(read_ag_buf);
	if(btree_buf_data) free(btree_buf_data);
	if(ptr) free(ptr);
	return rc;
}


int xfs_iter_blocks(const char *dev, 
	int (*callback)(int fd, uint64_t length,	uint64_t offset)) {
	
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

	sbp = libxfs_readbuf(xargs.ddev, XFS_SB_DADDR, 1, 0);

	memset(&mbuf, 0, sizeof(xfs_mount_t));
	super = &mbuf.m_sb;
	libxfs_sb_from_disk(super, XFS_BUF_TO_SBP(sbp));
	mp = libxfs_mount(&mbuf, super, xargs.ddev, xargs.logdev, xargs.rtdev, 1);

	if(!mp || mp->m_sb.sb_inprogress
		|| !mp->m_sb.sb_logstart || mp->m_sb.sb_rextents) {
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
	 
	first_agbno = (((XFS_AGFL_DADDR(mp) + 1) * sector_size) + first_residue)
		/ block_size;
	ag_count = mp->m_sb.sb_agcount;

	struct xfs_device_info devinfo;
	devinfo.dev = (char*) dev;
	devinfo.fd = fd;
	devinfo.mp = mp;
	devinfo.block_size = block_size;
	devinfo.sector_size = sector_size;
	devinfo.first_agbno = first_agbno;

	for(xfs_agnumber_t agno = 0; agno < ag_count; ++agno) {
		rc += xfs_iter_allocation_group(agno, &devinfo, callback);
	}

out:
	libxfs_device_close(xargs.ddev);  
	if(fd >= 0) {
		close(fd);
	}
	return rc;
}
