#include "test/loop_device.h"

#include <errno.h>
#include <linux/fs.h>
#include <fcntl.h>
#include <fstream>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace datto_linux_client_test {

LoopDevice::LoopDevice() {
  int fd;
  int create_ret = system("./test/make_test_loop_device");
  if (create_ret) {
    throw std::runtime_error("Couldn't make loop device");
  }

  std::ifstream loop_path_stream(TEST_LOOP_SHARED_MEMORY);
  std::getline(loop_path_stream, path_);
  loop_path_stream.close();

  if ((fd = open(path_.c_str(), O_RDONLY)) == -1) {
    PLOG(ERROR) << "Unable to make test loop device."
                << " Verify everything is cleaned up with losetup";
    unlink(TEST_LOOP_SHARED_MEMORY);
    throw std::runtime_error("Couldn't make loop device");
  }

  // BLKBSZGET ioctl doesn't write 0s to the most significant bits,
  // so we need to zero block_size_ first
  block_size_ = 0;
  if (ioctl(fd, BLKBSZGET, &block_size_)) {
    PLOG(ERROR) << "BLKBSZGET";
    throw std::runtime_error("Error getting block size");
  }

  if (close(fd)) {
    PLOG(ERROR) << "Error closing loop device";
  }
}

void LoopDevice::FormatAsExt3() {
  if (system(("mkfs.ext3 " + path_ + " 2>&1 1>/dev/null").c_str())) {
    throw std::runtime_error("error creating fs");
  }
}

void LoopDevice::Sync() {
  int fd;
  if ((fd = open(path_.c_str(), O_RDONLY)) == -1) {
    throw std::runtime_error("Unable to open loop device");
  }

  int ioctl_ret = ioctl(fd, BLKFLSBUF, 0);
  close(fd);

  if (ioctl_ret) {
    throw std::runtime_error("Unable to flush block device");
  }
}

LoopDevice::~LoopDevice() {
  sync();
  system(("losetup -d " + path_).c_str());
  unlink(TEST_LOOP_SHARED_MEMORY);
  system("rm /tmp/test_loop_file.* 2>/dev/null");
}

}
