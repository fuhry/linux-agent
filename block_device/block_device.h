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

  // Read a block using the normal mechanisms
  virtual void Read(off_t byte_offset, void *buf, int count);

  // Write a block using the normal mechanisms
  virtual void Write(off_t byte_offset, const void *buf, int count);

  // This will skip the page cache when reading
  //
  // Note that buf must be sector aligned and count must
  // be a multiple of the sector size.
  virtual void RawRead(off_t byte_offset, void *buf, int count);

  // This will be called automatically by RawRead if needed
  void BindRaw();

  // Flushes using the BLKFLSBUF ioctl
  virtual void Flush();

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
  std::string raw_path_;
  int fd_;
  int raw_fd_;

  // Do the actual initialization of the object
  void Init();

 private:
  void UnbindRaw();
  ::dev_t dev_t_;

  uint64_t device_size_bytes_;
  uint32_t block_size_bytes_;

  double throttle_scalar_;
};

}

#endif //  DATTO_CLIENT_BLOCK_DEVICE_BLOCK_DEVICE_H_
