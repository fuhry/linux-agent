#ifndef DATTO_CLIENT_BLOCK_DEVICE_DM_SNAPSHOTABLE_BLOCK_DEVICE_H_
#define DATTO_CLIENT_BLOCK_DEVICE_DM_SNAPSHOTABLE_BLOCK_DEVICE_H_

#include <boost/shared_ptr.hpp>
#include "block_device/snapshotable_block_device.h"

namespace datto_linux_client {

static const char DUPLICATE_POSTFIX[] = "_datto_dup";
static const char SNAPSHOT_POSTFIX[] = "_datto_snap";

class DmSnapshotableBlockDevice : public SnapshotableBlockDevice {
 public:
  DmSnapshotableBlockDevice(std::string block_path,
                            boost::shared_ptr<BlockDevice> cow_device);
  DmSnapshotableBlockDevice(uint32_t major,
                            uint32_t minor,
                            boost::shared_ptr<BlockDevice> cow_device);
  std::auto_ptr<DmSnapshot> TakeSnapshot();
  ~DmSnapshotableBlockDevice();
 private:
  boost::shared_ptr<BlockDevice> cow_device_;
};
}

#endif //  DATTO_CLIENT_BLOCK_DEVICE_DM_SNAPSHOTABLE_BLOCK_DEVICE_H_
