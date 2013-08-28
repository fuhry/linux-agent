#ifndef DATTO_CLIENT_SNAPSHOTABLE_BLOCK_DEVICE_H_
#define DATTO_CLIENT_SNAPSHOTABLE_BLOCK_DEVICE_H_

#include "block_device.h"
#include <memory>

namespace datto_linux_client {
class SnapshotableBlockDevice : public BlockDevice {
 public:
  virtual std::auto_ptr<Snapshot> TakeSnapshot() = 0;
  virtual ~SnapshotableBlockDevice();
};
}

#endif //  DATTO_CLIENT_SNAPSHOTABLE_BLOCK_DEVICE_H_
