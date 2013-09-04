#include "block_device/dm_snapshotable_block_device.h"
#include "block_device/dm_snapshot.h"
#include "block_device/device_mapper/*.h"

namespace datto_linux_client {

DmSnapshotableBlockDevice::DmSnapshotableBlockDevice(
    std::string block_path,
    boost::shared_ptr<BlockDevice> cow_device)
    : BlockDevice(block_path),
      cow_device_(cow_device) {

  // TODO: checks on block_path name + postfix lengths
  // TODO: check the cow_device exists and is writable
}

DmSnapshotableBlockDevice::DmSnapshotableBlockDevice(
    uint32_t major,
    uint32_t minor,
    boost::shared_ptr<BlockDevice> cow_device)
    : BlockDevice(major, minor),
      cow_device_(cow_device) {

  // TODO: checks on block_path name + postfix lengths
  // TODO: check the cow_device exists and is writable
}

std::auto_ptr<DmSnapshot> DmSnapshotableBlockDevice::TakeSnapshot() {
  // TODO, this function is the one that actually does all of the
  // device mapper work
  return std::unique_ptr<DmSnapshot>();
}
