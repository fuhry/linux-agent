#ifndef DATTO_CLIENT_BLOCK_DEVICE_XFS_MOUNTABLE_BLOCK_DEVICE_H_
#define DATTO_CLIENT_BLOCK_DEVICE_XFS_MOUNTABLE_BLOCK_DEVICE_H_

#include "block_device/mountable_block_device.h"

namespace datto_linux_client {

class XfsMountableBlockDevice : public MountableBlockDevice {
 public:
  explicit XfsMountableBlockDevice(std::string path);
  virtual std::unique_ptr<const SectorSet> GetInUseSectors();
};

}
#endif //  DATTO_CLIENT_BLOCK_DEVICE_XFS_MOUNTABLE_BLOCK_DEVICE_H_
