//
//  block_device.cc: implementation of the BlockDevice class
//

#include <block_device/block_device.h>

#include <fstream>
#include <string>
#include <cstdlib>
#include <cstdio>

#include <sys/types.h>
#include <sys/stat.h>
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

    const std::string devliteral = "/dev/";
    const std::string sysblock_lit = "/sys/block/";
    const std::string rotational_lit = "/queue/rotational";
    const std::string size_lit = "/size";
    const std::string blksize_lit = "/queue/physical_block_size";

    std::string blockdevname;
    std::string drivename;
    std::string filename;
    std::string inbuf;

    // blockdevname = block_path_;  // Copy to local storage for notational convenience

    if (block_path_.substr(0,5) != devliteral) {  // bail with exception if not a '/dev/' path
      std::string err = std::string("Error: ") + 
        block_path_ + 
        std::string(" not valid.. does not begin with \"/dev/\"");
      throw BlockDeviceException(err);
    }

    drivename = block_path_.substr(5, 3);  // Extract the drive designator ("sda", "hdb", etc.)

    if (stat(block_path_.c_str(), &statbuf) < 0) {  // bail with exception if stat() fails
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

    // Determine does_seek_ value

    filename = sysblock_lit + drivename + rotational_lit;  

    long rotational = read_long_(filename);;

    does_seek_ = (bool) rotational;

    // Get device block size

    filename = sysblock_lit + drivename + blksize_lit;  

    long blksize = read_long_(filename);
    block_size_bytes_ = blksize;

    // Get device size (in blocks)

    filename = sysblock_lit + drivename + size_lit;  

    long size = read_long_(filename);
    device_size_bytes_ = size * blksize;

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



  long BlockDevice::read_long_(std::string &filepath) const {

    std::ifstream f;

    f.open(filepath.c_str());

    if (! f.is_open()) {
      std::string err = std::string("Error opening ") + filepath + std::string(" for input: ");
      throw BlockDeviceException(err);
    }

    long tval;

    f >> tval;

    f.close();

    return tval;

  }


 }  //namespace   



    
