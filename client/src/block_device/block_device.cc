//
//  block_device.cc: implementation of the BlockDevice class
//

#include <block_device/block_device.h>

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

    const std::string DEV_LIT = "/dev/";

    std::string block_dev_name;

    // block_dev_name = block_path_;  // Copy to local storage for notational convenience

    if (block_path_.substr(0,DEV_LIT.length()) != DEV_LIT) {  // bail with exception if not a '/dev/' path
      std::string err = std::string("Error: ") + 
        block_path_ + 
        std::string(" not valid.. does not begin with \"/dev/\"");
      throw BlockDeviceException(err);
    }

    //  Note:  using lstat() instead of stat() to cause symlinks to block devices to fail

    if (lstat(block_path_.c_str(), &statbuf) < 0) {  // bail with exception if stat() fails
      std::string err = std::string("Error: could not stat() ") + block_path_;
      throw BlockDeviceException(err);
    }

    if (! S_ISBLK(statbuf.st_mode) ) {  // bail with exception if not a block device
      std::string err = std::string("Error: ") +
      block_path_ +
      std::string(" is not a block device");
      throw BlockDeviceException(err);
    }
    
    major_ = ::major(statbuf.st_rdev);  // Record major and minor numbers
    minor_ = ::minor(statbuf.st_rdev);

    int fd = 0;
    fd = open(block_path_.c_str(),   //  Open readonly here, just for ioctl() purposes
                  O_RDONLY | O_LARGEFILE);

    if (fd < 0) {
      std::string err = std::string("Error opening ") +
      block_path_ +
      std::string(" read-only for ioctl() calls\n");
      throw BlockDeviceException(err);
    }

    uint16_t rotational;

    ioctl(fd, BLKROTATIONAL, &rotational);
    ioctl(fd, BLKGETSIZE64, &device_size_bytes_);
    ioctl(fd, BLKSSZGET, &block_size_bytes_);

    close(fd);

    does_seek_ = rotational;    // short -> bool here

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
      std::string err = "Error: block device " + block_path_ + 
        " already open";
      throw BlockDeviceException(err);
    }

    int fd = open(block_path_.c_str(),
                  O_RDWR | O_LARGEFILE);

    if (fd < 0) {
      char * error_chars = strerror(errno);
      std::string err = std::string("Error opening ") +
                   block_path_ +
                   std::string("; error: ") +
                   std::string(error_chars);
      throw BlockDeviceException(err);
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



    
