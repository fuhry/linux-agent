#include "fs_parsing/ext_file_system.h"

#include "block_device/block_device_exception.h"

#include <fcntl.h>
#include <unistd.h>

#include <glog/logging.h>

namespace datto_linux_client {

ExtFileSystem::ExtFileSystem(std::string dev_path,
                             const ExtErrorTable &error_table)
    : fs_(NULL) {

  dev_fd_ = open(dev_path.c_str(), O_RDONLY);
  if (dev_fd_ < 0) {
    PLOG(ERROR) << "Unable to open device " << dev_path;
    throw BlockDeviceException("Unable to open device");
  }

  int rc;
  if ((rc = ext2fs_open(dev_path.c_str(), 0, 0, 0,
                        unix_io_manager, &fs_))) {
    PLOG(ERROR) << "Unable to open as ext device: "
                << error_table.get_error(rc);
    close(dev_fd_);
    throw BlockDeviceException("Unable to open device as ext");
  }

  if ((rc = ext2fs_read_bitmaps(fs_))) {
    PLOG(ERROR) << "Unable to read bitmap" << error_table.get_error(rc);
    ext2fs_close(fs_);
    close(dev_fd_);
    throw BlockDeviceException("Unable to read bitmap");
  }
}

ExtFileSystem::~ExtFileSystem() {
  ext2fs_close(fs_);
  close(dev_fd_);
}

} // datto_linux_client
