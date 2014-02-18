#include "test/loop_device.h"

#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include <linux/fs.h>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace datto_linux_client_test {

LoopDevice::LoopDevice() : is_backing_path_(false) {
  SetRandNum();
  int create_ret = system(("./test/make_test_loop_device " +
                           GetSharedMemoryPath()).c_str());
  if (create_ret) {
    throw std::runtime_error("Couldn't make loop device");
  }

  Init();
}

LoopDevice::LoopDevice(std::string backing_file_path)
    : is_backing_path_(true) {
  SetRandNum();
  int losetup_ret = system(("losetup -fv " + backing_file_path +
                            " | sed -e 's/Loop device is //g' > " +
                            GetSharedMemoryPath()).c_str());
  if (losetup_ret) {
    throw std::runtime_error("Couldn't make loop device");
  }

  Init();
}

// Use the address of this object in memory to seed the random number generator
void LoopDevice::SetRandNum() {
  auto unique = this;
  rand_num_ = rand_r((unsigned int *)&unique);
}

std::string LoopDevice::GetSharedMemoryPath() {
  return std::string(TEST_LOOP_SHARED_MEMORY) + "." +
         std::to_string(rand_num_);
}

void LoopDevice::Init() {
  std::ifstream loop_path_stream(GetSharedMemoryPath());
  std::getline(loop_path_stream, path_);
  loop_path_stream.close();

  int fd;
  if ((fd = open(path_.c_str(), O_RDONLY)) == -1) {
    PLOG(ERROR) << "Unable to make test loop device."
                << " Verify everything is cleaned up with losetup."
                << " path is: " << path_;
    throw std::runtime_error("Couldn't make loop device");
  }

  // BLKBSZGET ioctl doesn't write 0s to the most significant bits,
  // so we need to zero block_size_ first
  block_size_ = 0;
  if (ioctl(fd, BLKBSZGET, &block_size_)) {
    PLOG(ERROR) << "BLKBSZGET";
    close(fd);
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

void LoopDevice::FormatAsXfs() {
  if (system(("mkfs.xfs " + path_ + " 2>&1 1>/dev/null").c_str())) {
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
  int suppress_warning;
  // Sleep gives the kernel a chance to know any open descriptors are closed
  if (is_backing_path_) {
    suppress_warning = system(("sleep .25 && losetup -d " + path_).c_str());
  } else {
    suppress_warning =
      system(("sleep .25 && ./test/cleanup_test_loop_device " + path_).c_str());
  }
  (void) suppress_warning;

  // Remove the shared memory file
  unlink(GetSharedMemoryPath().c_str());
}

}
