#ifndef DATTO_CLIENT_FS_PARSING_EXT_MOUNTABLE_BLOCK_DEVICE_H_
#define DATTO_CLIENT_FS_PARSING_EXT_MOUNTABLE_BLOCK_DEVICE_H_

#include <memory>
#include "block_device/mountable_block_device.h"

#include <ext2fs/ext2fs.h>

namespace datto_linux_client {

class ExtMountableBlockDevice : public MountableBlockDevice {
 public:
  explicit ExtMountableBlockDevice(std::string block_path);
  virtual std::unique_ptr<const SectorSet> GetInUseSectors();

};

} // datto_linux_client
#endif //  DATTO_CLIENT_FS_PARSING_EXT_MOUNTABLE_BLOCK_DEVICE_H_
