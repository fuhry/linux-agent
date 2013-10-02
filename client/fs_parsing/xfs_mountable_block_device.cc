#include "fs_parsing/xfs_mountable_block_device.h"

#include <stdbool.h>
#include <error.h>
#include <endian.h>
extern "C" {
#define delete CXX_NO_DELETE_KEYWORD
#include "libxfs/include/libxfs.h"
#undef delete
}

#include "fs_parsing/tools.h"

// TODO consts
#define MAX_WRITE_BLOCK_LENGTH 0x100000
#define JOURNAL_LOG_LENGTH 0x100000

namespace datto_linux_client {

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
    struct xfs_btree_block *current_block;
  };

  // TODO Fix statics
  // TODO Whitespace
  // TODO Scoping
  // TODO Line length
  /** Handles an allocation group (ag)'s record data */
  static int xfs_handle_allocation_group_record(int agno,
                                struct xfs_device_info *devinfo,
                                xfs_daddr_t ag_begin,
                                xfs_daddr_t ag_end,
                                SectorSet *sectors)
  {

    int rc = 0;
    int write_block_length = 0;
    uint16_t num_records = be16toh(devinfo->current_block->bb_numrecs);
    uint64_t sizeb;
    uint64_t to_read;
    xfs_off_t write_position;

    //Get record pointer for block
    xfs_alloc_rec_t *record_ptr = XFS_ALLOC_REC_ADDR(devinfo->mp,
                                                     devinfo->current_block, 1); 

    //Iterate over all records, and read fully with callback
    for (int i = 0; i < num_records; ++i, ++record_ptr) {
        sizeb = XFS_AGB_TO_DADDR(devinfo->mp, agno, 
                                 be32toh(record_ptr->ar_startblock)) - ag_begin;

        to_read = my_roundup(sizeb << BBSHIFT, devinfo->sector_size);

        write_position = (xfs_off_t) ag_begin << BBSHIFT;
        while (to_read > 0) {
            if (to_read > MAX_WRITE_BLOCK_LENGTH) {
                write_block_length = MAX_WRITE_BLOCK_LENGTH;
                to_read -= MAX_WRITE_BLOCK_LENGTH;
            } else {
                write_block_length = to_read;
                to_read = 0;
            }

            if (lseek(devinfo->fd, write_position, SEEK_SET) < 0) {
                error(0, errno, "Error seeking %s (w position: %ld)", 
                            devinfo->dev, write_position);
                return rc;
            }

            sectors->insert(write_position);
            rc += write_block_length;

            if (lseek(devinfo->fd, write_block_length, SEEK_CUR) < 0) {
                error(0, errno, "Error seeking %s (w length: %d)",
                            devinfo->dev, write_block_length);
                return rc;
            }
            write_position += write_block_length;
        }
    }
    return rc;
  }


  /** Handles an allocation group (ag)'s btree data */
  static int xfs_handle_allocation_group_btree(int agno,
                                struct xfs_device_info *devinfo,
                                xfs_daddr_t ag_begin,
                                xfs_daddr_t ag_end,
                                SectorSet *sectors)
  {
    int rc = 0;
    int write_block_length = 0;
    uint32_t bb_rightsib = be32toh(devinfo->current_block->bb_u.s.bb_rightsib);
    uint64_t sizeb;
    uint64_t to_read;
    xfs_off_t write_position;
    void *btree_buf_data = NULL;

    if (bb_rightsib == NULLAGBLOCK) { //Are we at the end of the btree?
        return -1; //This will cause the caller to terminate
    }

    btree_buf_data = malloc(devinfo->block_size);
    // Locate the btree's first block and callback on it
    xfs_off_t btree_buf_position = (xfs_off_t) XFS_AGB_TO_DADDR(devinfo->mp,
                                                                agno,
                                                                be32toh(devinfo->current_block->bb_u.s.bb_rightsib)) << BBSHIFT;

    if (lseek(devinfo->fd, btree_buf_position, SEEK_SET) < 0) {
        error(0, errno, "Error seeking %s (btree pos: %ld)", devinfo->dev,
                    btree_buf_position);
        if (btree_buf_data) {
          free(btree_buf_data);
        }
        return rc;
    }

    sectors->insert(btree_buf_position);
    rc += devinfo->block_size;

    if (read(devinfo->fd, btree_buf_data, devinfo->block_size) < 0) {
        error(0, errno, "Error reading %s (btree pos: %ld)", devinfo->dev,
                    btree_buf_position);
        if (btree_buf_data) {
          free(btree_buf_data);
        }
        return rc;
    }

    //Set the current block to the next block
    devinfo->current_block = (struct xfs_btree_block*) btree_buf_data;

    if (ag_begin < ag_end) {
        sizeb = ag_end - ag_begin;
        to_read = my_roundup(sizeb << BBSHIFT, devinfo->sector_size);
        write_position = (xfs_off_t) ag_begin << BBSHIFT;

        while (to_read > 0) {
            if (to_read > MAX_WRITE_BLOCK_LENGTH) {
                write_block_length = MAX_WRITE_BLOCK_LENGTH;
                to_read -= MAX_WRITE_BLOCK_LENGTH;
            } else {
                write_block_length = to_read;
                to_read = 0;
            }

            if (lseek(devinfo->fd, write_position, SEEK_SET) < 0) {
                error(0, errno, "Error seeking %s (w pos: %ld)",
                            devinfo->dev, write_position);
                if (btree_buf_data) {
                  free(btree_buf_data);
                }
                return rc;
            }

            sectors->insert(write_position);
            rc += write_block_length;

            if (lseek(devinfo->fd, write_block_length, SEEK_CUR) < 0) {
                error(0, errno, "Error seeking %s (w length: %d)",
                            devinfo->dev, write_block_length);
                if (btree_buf_data) {
                  free(btree_buf_data);
                }
                return rc;
            }
            write_position += write_block_length;
        }
    }
    if (btree_buf_data) {
      free(btree_buf_data);
    }
    return rc;
  }


  /** Handles an allocation group (ag)'s journal data */
  static int xfs_handle_allocation_group_journal(struct xfs_device_info *devinfo,
                                                 SectorSet *sectors)
  {
    int rc = 0;
    int journal_log_start = XFS_FSB_TO_DADDR(devinfo->mp,
                                             devinfo->mp->m_sb.sb_logstart);
    journal_log_start = journal_log_start << BBSHIFT;

    int journal_log_start_pos = my_rounddown(journal_log_start,
                                             (xfs_off_t) JOURNAL_LOG_LENGTH);

    if (journal_log_start % JOURNAL_LOG_LENGTH) {
        if (lseek(devinfo->fd, journal_log_start_pos, SEEK_SET) < 0) {
            error(0, errno, "Error seeking %s (journal_log_start_pos: %d)",
                        devinfo->dev, journal_log_start_pos);
            return rc;
        }

        sectors->insert(journal_log_start_pos);
        rc += JOURNAL_LOG_LENGTH;

        if (lseek(devinfo->fd, JOURNAL_LOG_LENGTH, SEEK_CUR) < 0) {
            error(0, errno, "Error seeking %s (loglength: %d)",
                        devinfo->dev, JOURNAL_LOG_LENGTH);
            return rc;
        }
    }

    int journal_log_end = (XFS_FSB_TO_DADDR(devinfo->mp, devinfo->mp->m_sb.sb_logstart) << BBSHIFT) +
                           XFS_FSB_TO_B(devinfo->mp, devinfo->mp->m_sb.sb_logblocks);

    int journal_log_end_pos = my_rounddown(journal_log_end,
                                           (xfs_off_t) JOURNAL_LOG_LENGTH);

    if (journal_log_end % JOURNAL_LOG_LENGTH) {
        if (lseek(devinfo->fd, journal_log_end_pos, SEEK_SET) < 0) {
            error(0, errno, "Error seeking %s (journal_log_start_pos: %d)",
                        devinfo->dev, journal_log_start_pos);
            return rc;
        }

        sectors->insert(journal_log_end_pos);
        rc += JOURNAL_LOG_LENGTH;

        if (lseek(devinfo->fd, JOURNAL_LOG_LENGTH, SEEK_CUR) < 0) {
            error(0, errno, "Error seeking %s (loglength: %d)", 
                        devinfo->dev, JOURNAL_LOG_LENGTH);
            return rc;
        }
    }
    return rc;
  }


  /** Parses blocks with in an allocation group (records, btree, journal, etc) */
  static int xfs_iter_allocation_group_blocks(int agno, xfs_daddr_t ag_begin,
                                xfs_daddr_t ag_end,
                                struct xfs_device_info *devinfo,
                                SectorSet *sectors)
  {

    int rc = 0;
    int btree_rc = 0;
    while (true) {
        /* Handle records */
        rc += xfs_handle_allocation_group_record(agno, devinfo,
                                        ag_begin, ag_end, sectors);

        /* Handle btree */
        btree_rc = xfs_handle_allocation_group_btree(agno, devinfo,
                                        ag_begin, ag_end, sectors);
        if (btree_rc == -1) { //Was I told to break?
            break;
        } else {
            rc += btree_rc;
        }

        /* Handle journaling */
        rc += xfs_handle_allocation_group_journal(devinfo, sectors);
    }

    return rc;
  }


  /** Parse a given allocation group then call function to read the contents */
  static int xfs_iter_allocation_group(int agno, struct xfs_device_info *devinfo,
                                       SectorSet *sectors)
  {
    int rc = 0;
    ag_header_t ag_hdr;
    xfs_daddr_t read_ag_off;
    int read_ag_length;
    void *read_ag_buf = NULL;
    xfs_off_t read_ag_position;

    void *btree_buf_data = NULL;
    xfs_off_t btree_buf_position;

    xfs_agblock_t bno;
    xfs_daddr_t ag_begin = 0;
    xfs_daddr_t ag_end = 0;
    xfs_alloc_ptr_t *ptr = NULL;

    read_ag_off = XFS_AG_DADDR(devinfo->mp, agno, XFS_SB_DADDR);
    read_ag_length = devinfo->first_agbno * devinfo->block_size;
    read_ag_position = (xfs_off_t) read_ag_off * (xfs_off_t) BBSIZE;
    read_ag_buf = malloc(read_ag_length);

    memset(read_ag_buf, 0, read_ag_length);

    if (lseek(devinfo->fd, read_ag_position, SEEK_SET) < 0) {
        error(0, errno, "Error seeking %s (ag position: %ld)",
                    devinfo->dev, read_ag_position);
        goto out;
    }

    sectors->insert(read_ag_position);
    rc += read_ag_length;

    if (read(devinfo->fd, read_ag_buf, read_ag_length) < 0) {
        error(0, errno, "Error reading %s (ag position: %ld)",
                    devinfo->dev, read_ag_position);
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

    bno = be32toh(ag_hdr.xfs_agf->agf_roots[XFS_BTNUM_BNOi]);
    ag_end = XFS_AGB_TO_DADDR(devinfo->mp, agno,
                              be32toh(ag_hdr.xfs_agf->agf_length) - 1) +
                                devinfo->block_size / BBSIZE;

    for (uint32_t current_level = 1;; ++current_level) {
        uint16_t bb_level;
        btree_buf_position = (xfs_off_t) XFS_AGB_TO_DADDR(devinfo->mp, agno, bno)
                                                 << BBSHIFT;

        if (lseek(devinfo->fd, btree_buf_position, SEEK_SET) < 0) {
            error(0, errno, "Error seeking %s (btree position: %ld)", 
                        devinfo->dev, btree_buf_position);
            goto out;
        }

        sectors->insert(btree_buf_position);
        rc += devinfo->block_size;

        if (read(devinfo->fd, btree_buf_data, devinfo->block_size) < 0) {
            error(0, errno, "Error reading %s (btree position: %ld)",
                        devinfo->dev, btree_buf_position);
            goto out;
        }
        devinfo->current_block = (struct xfs_btree_block*) btree_buf_data;
        bb_level = be16toh(devinfo->current_block->bb_level);

        if (bb_level == 0) {
            break;
        }

        ptr = XFS_ALLOC_PTR_ADDR(devinfo->mp, devinfo->current_block, 1,
                                 devinfo->mp->m_alloc_mxr[1]);

        bno = be32toh(ptr[0]);
    }

    ag_begin = (read_ag_position >> BBSHIFT) + (read_ag_length >> BBSHIFT);

    rc += xfs_iter_allocation_group_blocks(agno, ag_begin, ag_end, devinfo, sectors);

out:
    if (read_ag_buf) {
        free(read_ag_buf);
    }
    if (btree_buf_data) {
        free(btree_buf_data);
    }
    if (ptr) {
        free(ptr);
    }
    return rc;
  }


  XfsMountableBlockDevice::XfsMountableBlockDevice(std::string block_path)
      : MountableBlockDevice(block_path) { }
  
  std::unique_ptr<const SectorSet> XfsMountableBlockDevice::GetInUseSectors() {
    SectorSet *sectors = new SectorSet();
    XfsMountableBlockDevice::xfs_iter_blocks(sectors);    
    return std::unique_ptr<const SectorSet>(sectors);
  }
  
  int XfsMountableBlockDevice::xfs_iter_blocks(SectorSet *sectors) {
    const char *dev = BlockDevice::block_path().c_str();
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
    if (fd < 0) {
        error(0, errno, "Error opening %s", dev);
        goto out;
    }

    memset(&xargs, 0, sizeof(xargs));
    xargs.isdirect = LIBXFS_DIRECT;
    xargs.isreadonly = LIBXFS_ISREADONLY;
    xargs.volname = (char*) dev;
    if (!libxfs_init(&xargs)) {
        error(0, errno, "Error initializing xfs partition %s", dev);
        goto out;
    }

    sbp = libxfs_readbuf(xargs.ddev, XFS_SB_DADDR, 1, 0);

    memset(&mbuf, 0, sizeof(xfs_mount_t));
    super = &mbuf.m_sb;
    libxfs_sb_from_disk(super, XFS_BUF_TO_SBP(sbp));
    mp = libxfs_mount(&mbuf, super, xargs.ddev, xargs.logdev, xargs.rtdev, 1);

    if (!mp || mp->m_sb.sb_inprogress
            || !mp->m_sb.sb_logstart
            || mp->m_sb.sb_rextents) {
        error(0, errno, "Failed to initialize %s", dev);
        goto out;
    }

    block_size = mp->m_sb.sb_blocksize;
    sector_size = mp->m_sb.sb_sectsize;

    if (block_size > sector_size)    {
        int leftover = ((XFS_AGFL_DADDR(mp) + 1) * sector_size) % block_size;
        first_residue = (leftover == 0) ? 0 :    block_size - leftover;
    } else if (block_size == sector_size) {
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
    devinfo.current_block = 0;

    for (xfs_agnumber_t agno = 0; agno < ag_count; ++agno) {
        rc += xfs_iter_allocation_group(agno, &devinfo, sectors);
    }

out:
    libxfs_device_close(xargs.ddev);
    if (fd >= 0) {
        close(fd);
    }
    return rc;
  }
}
