#ifndef DATTO_CLIENT_BLOCK_DEVICE_MOUNTABLE_BLOCK_DEVICE_H_
#define DATTO_CLIENT_BLOCK_DEVICE_MOUNTABLE_BLOCK_DEVICE_H_

#include <memory>
#include <string>

#include <stdint.h>
#include <time.h>

#include "block_device/block_device.h"
#include "unsynced_sector_manager/sector_set.h"

namespace datto_linux_client {

class MountableBlockDevice : public BlockDevice {
 public:
  explicit MountableBlockDevice(std::string a_path);

  virtual bool IsMounted() const;
  // Throw an exception if the partition isn't mounted
  virtual std::string GetMountPoint() const;

  // These should only be overwritten if the underlying filesystem
  // doesn't support the FIFREEZE ioctl
  //
  // Before calling these the MountableBlockDevice must not have an open
  // file descriptor for the mount point (i.e., call CloseMount() first)
  virtual void Freeze();
  virtual void Thaw();

  // These will be relative from the start of the partition
  virtual std::shared_ptr<const SectorSet> GetInUseSectors() = 0;

  // Return a file descriptor for the mount point
  // Throw an exception if one is already open, or if
  // it isn't mounted
  int OpenMount();

  // Close the file descriptor returned by OpenMount()
  // Don't throw if one isn't open
  void CloseMount();

  virtual std::string GetUuid() const;

  // Should close the mount file descriptor and unfreeze
  virtual ~MountableBlockDevice();

 protected:
  // For unit testing
  MountableBlockDevice() : mount_file_descriptor_(-1),
                           uuid_(), is_frozen_(false) {}

 private:
  int mount_file_descriptor_;
  std::string uuid_;
  bool is_frozen_;
  // Used to cache the IsMounted check so it can be used in a loop
  mutable time_t last_mount_check_time_;
  mutable bool last_mounted_result_;
};

}

#endif //  DATTO_CLIENT_BLOCK_DEVICE_MOUNTABLE_BLOCK_DEVICE_H_
