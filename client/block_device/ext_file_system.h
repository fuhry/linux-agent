#ifndef DATTO_CLIENT_BLOCK_DEVICE_EXT_FILE_SYSTEM_H_
#define DATTO_CLIENT_BLOCK_DEVICE_EXT_FILE_SYSTEM_H_

#include <string>
#include <memory>

#include "block_device/ext_error_table-inl.h"
#include <ext2fs/ext2fs.h>

namespace datto_linux_client {

class ExtFileSystem {
 public:
  ExtFileSystem(std::string dev_path, const ExtErrorTable &error_table);

  // ext2_filesys is defined in ext2fs/ext2fs.h
  ext2_filsys fs() {
    return fs_;
  }

  struct ext2_super_block * super() {
    return fs_->super;
  }

  ext2fs_block_bitmap block_map() {
    return fs_->block_map;
  }

  dgrp_t group_desc_count() {
    return fs_->group_desc_count;
  }
  
  int dev_fd() {
    return dev_fd_;
  }

  ~ExtFileSystem();

 private:
  ext2_filsys fs_;
  int dev_fd_;
};

} // datto_linux_client

#endif // DATTO_CLIENT_BLOCK_DEVICE_EXT_FILE_SYSTEM_H_
