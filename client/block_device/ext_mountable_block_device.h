#ifndef DATTO_CLIENT_BLOCK_DEVICE_EXT_MOUNTABLE_BLOCK_DEVICE_H_
#define DATTO_CLIENT_BLOCK_DEVICE_EXT_MOUNTABLE_BLOCK_DEVICE_H_

#include <memory>
#include "block_device/mountable_block_device.h"

#include "unsynced_sector_manager/sector_set.h"

#include <ext2fs/ext2fs.h>

namespace datto_linux_client {

class ExtMountableBlockDevice : public MountableBlockDevice {
 public:
  explicit ExtMountableBlockDevice(std::string path, bool is_ext2);
  virtual std::shared_ptr<const SectorSet> GetInUseSectors();
  virtual void Freeze();
  virtual void Thaw();
 private:
  bool is_ext2_;

};

} // datto_linux_client
#endif //  DATTO_CLIENT_BLOCK_DEVICE_EXT_MOUNTABLE_BLOCK_DEVICE_H_
