//
//  block_device.cc: implementation of the BlockDevice class
//

#include "block_device/block_device.h"

#include <string>
#include <cstdlib>
#include <cstdio>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


namespace datto_linux_client {

BlockDevice::BlockDevice(std::string block_path) {
  block_path_ = block_path;
  Init();
}

void BlockDevice::Init() {
  struct stat statbuf;

  // Note: using lstat() instead of stat() to cause symlinks to
  // block devices to fail
  if (lstat(block_path_.c_str(), &statbuf) < 0) {
    std::string err = std::string("Error: could not stat() ") + block_path_;
    throw BlockDeviceException(err);
  }
  // bail with exception if not a block device
  if (! S_ISBLK(statbuf.st_mode) ) {
    throw BlockDeviceException("Error: " + block_path_ +
                               " is not a block device");
  }

  major_ = ::major(statbuf.st_rdev);
  minor_ = ::minor(statbuf.st_rdev);

  int fd = 0;
  fd = open(block_path_.c_str(), O_RDONLY | O_LARGEFILE);

  if (fd < 0) {
    throw BlockDeviceException("Error opening " + block_path_ +
                               " read-only for ioctl() calls\n");
  }

  ioctl(fd, BLKGETSIZE64, &device_size_bytes_);
  ioctl(fd, BLKSSZGET, &block_size_bytes_);

  close(fd);

  throttle_scalar_ = 0.0;
  file_descriptor_ = -1;
}

void BlockDevice::Throttle(double scalar) {
  throttle_scalar_ = scalar;
}

void BlockDevice::Unthrottle() {
  throttle_scalar_ = 0.0;;
}

int BlockDevice::Open() {
  if (file_descriptor_ != -1) {
    throw BlockDeviceException("Error: block device " + block_path_ +
                               " already open");
  }

  int fd = open(block_path_.c_str(),
      O_RDWR | O_LARGEFILE);

  if (fd < 0) {
    char * error_chars = strerror(errno);
    throw BlockDeviceException("Error opening " + block_path_ + "; error: " +
                               error_chars);
  }

  file_descriptor_ = fd;
  return fd;
}

void BlockDevice::Close() {
  if (file_descriptor_ > -1) {
    close(file_descriptor_);
  }
  file_descriptor_ = -1;
}

BlockDevice::~BlockDevice() {
  Close();
  Unthrottle();
}

}  //namespace   
