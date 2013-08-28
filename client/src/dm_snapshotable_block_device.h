#ifndef DATTO_CLIENT_DM_SNAPSHOTABLE_BLOCK_DEVICE_H_
#define DATTO_CLIENT_DM_SNAPSHOTABLE_BLOCK_DEVICE_H_

#include <boost/shared_ptr.hpp>
#include "snapshotable_block_device.h"

namespace datto_linux_client {
class DmSnapshotableBlockDevice : public SnapshotableBlockDevice {
 public:
  DmSnapshotableBlockDevice(std::string block_path,
                            boost::shared_ptr<BlockDevice> cow_device)
  DmSnapshotableBlockDevice(uint32_t major,
                            uint32_t minor,
                            boost::shared_ptr<BlockDevice> cow_device)
  std::auto_ptr<DmSnapshot> TakeSnapshot();
  ~DmSnapshotableBlockDevice();
 private:
  static std::string duplicate_postfix_ = "_datto_dup";
  static std::string snapshot_postfix_ = "_datto_snap";
  boost::shared_ptr<BlockDevice> cow_device_;
};
}

#endif //  DATTO_CLIENT_DM_SNAPSHOTABLE_BLOCK_DEVICE_H_
