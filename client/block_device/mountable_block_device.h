#ifndef DATTO_CLIENT_BLOCK_DEVICE_MOUNTABLEBLOCK_DEVICE_H_
#define DATTO_CLIENT_BLOCK_DEVICE_MOUNTABLEBLOCK_DEVICE_H_

#include <boost/noncopyable.hpp>
#include <boost/cstdint.hpp>
#include <memory>
#include <string>

#include "block_device/block_device.h"
#include "unsynced_sector_tracker/sector_set.h"

namespace datto_linux_client {

class MountableBlockDevice : public BlockDevice {

 public:
  explicit MountableBlockDevice(std::string block_path);

  virtual bool IsMounted();
  // Throw an exception if the partition isn't mounted
  virtual std::string GetMountPoint();

  // These should only be overwritten if the underlying filesystem
  // doesn't support the FIFREEZE ioctl
  // Note that the MountableBlockDevice must not have an open
  // file descriptor for the mount point.
  virtual void Freeze();
  virtual void Thaw();

  // These will be relative from the start of the partition, not from
  // the start of the volume
  virtual std::unique_ptr<const SectorSet> GetInUseSectors() = 0;

  // Return a file descriptor for the mount point
  // Throw an exception if one is already open
  int OpenMount();

  // Close the file descriptor returned by OpenMount()
  // Don't throw if one isn't open
  void CloseMount();

  // Should close the mount file descriptor and unfreeze
  virtual ~MountableBlockDevice();

 private:
  int mount_file_descriptor_;
};

}

#endif //  DATTO_CLIENT_BLOCK_DEVICE_MOUNTABLEBLOCK_DEVICE_H_
