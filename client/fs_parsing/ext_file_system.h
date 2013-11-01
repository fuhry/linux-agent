#ifndef DATTO_CLIENT_FS_PARSING_EXT_FILE_SYSTEM_H_
#define DATTO_CLIENT_FS_PARSING_EXT_FILE_SYSTEM_H_

#include <string>
#include <memory>

#include "fs_parsing/ext_error_table-inl.h"
#include <ext2fs/ext2fs.h>

namespace datto_linux_client {

class ExtFileSystem {
 public:
  ExtFileSystem(std::string dev_path, const ExtErrorTable &error_table);

  // ext2_filesys is defined in ext2fs/ext2fs.h
  ext2_filsys fs() {
    return fs_;
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

#endif // DATTO_CLIENT_FS_PARSING_EXT_FILE_SYSTEM_H_
