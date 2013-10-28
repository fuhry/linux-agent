#ifndef DATTO_CLIENT_FS_PARSING_EXT_FILE_SYSTEM_H_
#define DATTO_CLIENT_FS_PARSING_EXT_FILE_SYSTEM_H_

#include "fs_parsing/ext_error_table.h"
#include "block_device/block_device_exception.h"

#include <fcntl.h>
#include <unistd.h>

#include <glog/logging.h>

#include <ext2fs/ext2fs.h>

namespace datto_linux_client {

class ExtFileSystem {
 public:
  ExtFileSystem(std::string dev_path, const ExtErrorTable &error_table)
      : fs_(new struct struct_ext2_filsys()) {

    dev_fd_ = open(dev_path.c_str(), O_RDONLY);
    if (dev_fd_ < 0) {
      PLOG(ERROR) << "Unable to open device " << dev_path;
      throw BlockDeviceException("Unable to open device");
    }

    int rc;
    if ((rc = ext2fs_read_bitmaps(fs_.get()))) {
      PLOG(ERROR) << "Unable to read bitmap" << error_table.get_error(rc);
      close(dev_fd_);
      throw BlockDeviceException("Unable to read bitmap");
    }

    auto fs_ptr = fs_.get();

    // This needs to be the last step in the constructor, as the destructor
    // is responsible for cleaning up
    if ((rc = ext2fs_open(dev_path.c_str(), 0, 0, 0,
                          unix_io_manager, &fs_ptr))) {
      PLOG(ERROR) << "Unable to open as ext device: "
                  << error_table.get_error(rc);
      close(dev_fd_);
      throw BlockDeviceException("Unable to open device as ext");
    }
  }

  std::shared_ptr<struct struct_ext2_filsys> fs() {
    return fs_;
  }
  
  int dev_fd() {
    return dev_fd_;
  }

  ~ExtFileSystem() {
    ext2fs_close(fs_.get());
    close(dev_fd_);
  }

 private:
  std::shared_ptr<struct struct_ext2_filsys> fs_;
  int dev_fd_;
};
} // datto_linux_client

#endif // DATTO_CLIENT_FS_PARSING_EXT_FILE_SYSTEM_H_
