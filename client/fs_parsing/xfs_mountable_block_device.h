#ifndef DATTO_CLIENT_FS_PARSING_XFS_MOUNTABLE_BLOCK_DEVICE_H_
#define DATTO_CLIENT_FS_PARSING_XFS_MOUNTABLE_BLOCK_DEVICE_H_

#include "block_device/mountable_block_device.h"

namespace datto_linux_client {

class XfsMountableBlockDevice : public MountableBlockDevice {
 public:
  explicit XfsMountableBlockDevice(std::string block_path);
  virtual std::unique_ptr<const SectorSet> GetInUseSectors();
};

}
#endif //  DATTO_CLIENT_FS_PARSING_XFS_MOUNTABLE_BLOCK_DEVICE_H_
