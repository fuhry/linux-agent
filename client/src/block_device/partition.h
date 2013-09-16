#ifndef DATTO_CLIENT_BLOCK_DEVICE_PARTITION_H_
#define DATTO_CLIENT_BLOCK_DEVICE_PARTITION_H_

#include <boost/noncopyable.hpp>
#include <string>

#include "unsynced_block_tracker/sector_set.h"
#include "block_device/block_device.h"

namespace datto_linux_client {

class Partition : private boost::noncopyable {

 public:
  bool IsMounted();
  // Throw an exception if the partition isn't mounted
  std::string GetMountPoint();

  // These should only be overwritten if the underlying partition type
  // doesn't support the FIFREEZE ioctl
  virtual void Freeze();
  virtual void Unfreeze();

  // These will be relative from the start of the partition, not from
  // the start of the volume
  virtual std::unique_ptr<const SectorSet> GetInUseSectors() = 0;

  // Remember to unfreeze(!)
  virtual ~Partition();

 protected:
  // block_device will be an instance of /dev/sda1 or similar
  explicit Partition(std::unique_ptr<BlockDevice> block_device);
  std::unique_ptr<BlockDevice> block_device_;

};

}

#endif //  DATTO_CLIENT_BLOCK_DEVICE_PARTITION_H_
