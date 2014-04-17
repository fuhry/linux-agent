#ifndef DATTO_CLIENT_BLOCK_DEVICE_BLOCK_DEVICE_H_
#define DATTO_CLIENT_BLOCK_DEVICE_BLOCK_DEVICE_H_

#include <stdint.h>
#include <string>

#include <linux/types.h>

#include "block_device/block_device_exception.h"

namespace datto_linux_client {

class BlockDevice {

 public:
  // Creates a BlockDevice from the given block_path
  // An exception should be thrown (likely a BlockDeviceException) if the
  // block_path doesn't exist or isn't the path for a block_device
  explicit BlockDevice(std::string block_path);

  std::string path() const {
    return path_;
  }

  ::dev_t dev_t() const {
    return dev_t_;
  };

  // BLKGETSIZE64
  uint64_t DeviceSizeBytes() const {
    return device_size_bytes_;
  }

  uint64_t BlockSizeBytes() const {
    return block_size_bytes_;
  }

  // Return a file descriptor for the block device
  // Throw an exception if one is already open
  virtual int Open();

  // Flushes using the BLKFLSBUF ioctl
  virtual void Flush();

  // Close the file descriptor returned
  // Don't throw if one isn't open
  virtual void Close();

  // Should close the file descriptor
  virtual ~BlockDevice();

  BlockDevice(const BlockDevice &) = delete;
  BlockDevice& operator=(const BlockDevice &) = delete;

 protected:
  // Use this constructor when the block_path doesn't exist yet
  // Note that block_path_ must be set and Init() called before the
  // subclass constructor returns
  BlockDevice() { }
  std::string path_;

  // Do the actual initialization of the object
  void Init();

 private:
  ::dev_t dev_t_;

  uint64_t device_size_bytes_;
  uint32_t block_size_bytes_;

  double throttle_scalar_;

  int fd_;
};

}

#endif //  DATTO_CLIENT_BLOCK_DEVICE_BLOCK_DEVICE_H_
