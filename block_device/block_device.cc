#include "block_device/block_device.h"

#include <string>

#include <fcntl.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <glog/logging.h>

namespace datto_linux_client {

BlockDevice::BlockDevice(std::string a_path) : path_(a_path) {
  // Init() is also called by subclasses
  Init();
}

void BlockDevice::Init() {
  fd_ = -1;
  struct stat statbuf;

  // Note: using lstat() instead of stat() to cause symlinks to
  // block devices to fail
  if (lstat(path_.c_str(), &statbuf) < 0) {
    PLOG(ERROR) << "lstat() failed";
    throw BlockDeviceException("Unable to stat path");
  }
  // bail with exception if not a block device
  if (!S_ISBLK(statbuf.st_mode)) {
    PLOG(ERROR) << path_ << " is not a block device";
    throw BlockDeviceException("Not a block device");
  }

  dev_t_ = statbuf.st_rdev;

  int fd = open(path_.c_str(), O_RDONLY);

  if (fd < 0) {
    PLOG(ERROR) << "Open path: " << path_;
    throw BlockDeviceException("Error opening " + path_ +
                               " read-only for ioctl() calls\n");
  }

  ioctl(fd, BLKGETSIZE64, &device_size_bytes_);
  ioctl(fd, BLKBSZGET, &block_size_bytes_);
  close(fd);
}

void BlockDevice::Flush() {
  int fd = open(path_.c_str(), O_RDWR);

  if (ioctl(fd, BLKFLSBUF, 0)) {
    PLOG(ERROR) << "BLKFLSBUF " << path_;
  }

  close(fd);
}

int BlockDevice::Open() {
  if (fd_ != -1) {
    throw BlockDeviceException("Block device already open");
  }

  fd_ = open(path_.c_str(), O_RDWR | O_LARGEFILE);

  if (fd_ < 0) {
    PLOG(ERROR) << "Error opening " << path_;
    throw BlockDeviceException("Error opening block device");
  }

  return fd_;
}

void BlockDevice::Close() {
  close(fd_);
  fd_ = -1;
}

BlockDevice::~BlockDevice() {
  Close();
}

}  //namespace
