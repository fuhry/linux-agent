#include "block_device/block_device.h"

#include <string>

#include <fcntl.h>
#include <linux/fs.h>
#include <linux/raw.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <glog/logging.h>

namespace {
using datto_linux_client::BlockDeviceException;
static bool raw_loaded = false;
const char RAW_CTL[] = "/dev/raw/rawctl";
const char RAW_PREFIX[] = "/dev/raw/raw";

void LoadRawModule() {
  if (raw_loaded) {
    return;
  }

  struct stat buf;
  if (stat(RAW_CTL, &buf) == -1) {
    if (errno == ENOENT) {
      LOG(WARNING) << "Raw isn't compiled in or module isn't loaded";
      LOG(INFO) << "Attempting to load Raw module";
      if (system("modprobe raw") != 0) {
        LOG(ERROR) << "Unable to load Raw module";
        throw BlockDeviceException("Unable to load Raw module.");
      }
    } else {
      PLOG(ERROR) << "Unable to check existance of " << RAW_CTL;
    }
  }

  raw_loaded = true;
}
}

namespace datto_linux_client {

BlockDevice::BlockDevice(std::string a_path) : path_(a_path) {
  // Init() is also called by subclasses
  Init();
}

void BlockDevice::Init() {
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

  fd_ = open(path_.c_str(), O_RDWR);

  if (fd_ < 0) {
    PLOG(ERROR) << "Open path: " << path_;
    throw BlockDeviceException("Error opening " + path_ +
                               " read-only for ioctl() calls\n");
  }

  ioctl(fd_, BLKGETSIZE64, &device_size_bytes_);
  ioctl(fd_, BLKBSZGET, &block_size_bytes_);
}

void BlockDevice::Flush() {
  if (ioctl(fd_, BLKFLSBUF, 0)) {
    PLOG(ERROR) << "BLKFLSBUF " << path_;
  }
}

void BlockDevice::Read(off_t byte_offset, void *buf, int count) {
  ssize_t num_read = pread(fd_, buf, count, byte_offset);
  if (num_read != count) {
    PLOG(ERROR) << "Error reading from " << path_;
    throw BlockDeviceException("Error reading " + path_);
  }
}

void BlockDevice::Write(off_t byte_offset, const void *buf, int count) {
  ssize_t num_wrote = pwrite(fd_, buf, count, byte_offset);
  if (num_wrote != count) {
    PLOG(ERROR) << "Error writing from " << path_;
    throw BlockDeviceException("Error writing " + path_);
  }
}

void BlockDevice::RawRead(off_t byte_offset, void *buf, int count) {

  if (raw_fd_ == -1) {
    BindRaw();
  }

  ssize_t num_read = pread(raw_fd_, buf, count, byte_offset);
  if (num_read != count) {
    PLOG(ERROR) << "Error reading from " << raw_path_;
    throw BlockDeviceException("Error reading " + raw_path_);
  }
}

void BlockDevice::BindRaw() {
  LoadRawModule();
  int raw_ctl_fd = open(RAW_CTL, O_RDWR, 0);
  if (raw_ctl_fd < 0) {
    PLOG(ERROR) << "Unable to open " << RAW_CTL;
    throw BlockDeviceException("Error opening raw control");
  }
  struct raw_config_request rq;
  // FIXME This is not a good way to find a raw device
  // This is lazy and will cause issues if the user has other raw devices
  rq.raw_minor = 255 - minor(dev_t_);
  rq.block_minor = minor(dev_t_);
  rq.block_major = major(dev_t_);
  int err = ioctl(raw_ctl_fd, RAW_SETBIND, &rq);
  close(raw_ctl_fd);
  if (err < 0) {
    PLOG(ERROR) << "Error binding raw device with minor " << minor(dev_t_);
    throw BlockDeviceException("Error binding raw device");
  }

  raw_path_ = RAW_PREFIX + std::to_string(255 - minor(dev_t_));
  raw_fd_ = open(raw_path_.c_str(), O_RDONLY, 0);
  if (raw_fd_ < 0) {
    PLOG(ERROR) << "Error opening raw device " << raw_path_;
    throw BlockDeviceException("Error opening raw device");
  }
}

// This is called in the destructor so *do not throw*
void BlockDevice::UnbindRaw() {
  int raw_ctl_fd = open(RAW_CTL, O_RDWR, 0);
  struct raw_config_request rq;
  rq.raw_minor = minor(dev_t_);
  rq.block_minor = 0;
  rq.block_major = 0;
  ioctl(raw_ctl_fd, RAW_SETBIND, &rq);
  close(raw_ctl_fd);
}

BlockDevice::~BlockDevice() {
  close(raw_fd_);
  UnbindRaw();
  close(fd_);
}

}  //namespace
