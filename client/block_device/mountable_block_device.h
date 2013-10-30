#ifndef DATTO_CLIENT_BLOCK_DEVICE_MOUNTABLE_BLOCK_DEVICE_H_
#define DATTO_CLIENT_BLOCK_DEVICE_MOUNTABLE_BLOCK_DEVICE_H_

#include <memory>
#include <string>

#include <stdint.h>

#include "block_device/block_device.h"
#include "unsynced_sector_manager/sector_set.h"

namespace datto_linux_client {

class MountableBlockDevice : public BlockDevice {

 public:
  explicit MountableBlockDevice(std::string a_block_path);

  virtual bool IsMounted();
  // Throw an exception if the partition isn't mounted
  virtual std::string GetMountPoint();

  // These should only be overwritten if the underlying filesystem
  // doesn't support the FIFREEZE ioctl
  //
  // Before calling these the MountableBlockDevice must not have an open
  // file descriptor for the mount point (i.e., call CloseMount() first)
  virtual void Freeze();
  virtual void Thaw();

  // These will be relative from the start of the partition, not from
  // the start of the volume
  virtual std::unique_ptr<const SectorSet> GetInUseSectors() = 0;

  // Return a file descriptor for the mount point
  // Throw an exception if one is already open, or if
  // it isn't mounted
  int OpenMount();

  // Close the file descriptor returned by OpenMount()
  // Don't throw if one isn't open
  void CloseMount();

  // Should close the mount file descriptor and unfreeze
  virtual ~MountableBlockDevice();

 private:
  int mount_file_descriptor_;
  bool is_frozen_;
};

}

#endif //  DATTO_CLIENT_BLOCK_DEVICE_MOUNTABLEBLOCK_DEVICE_H_
