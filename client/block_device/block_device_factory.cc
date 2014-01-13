#include "block_device/block_device_factory.h"

#include "block_device/nbd_block_device.h"
#include "block_device/ext_mountable_block_device.h"
#include "block_device/xfs_mountable_block_device.h"

#include <blkid/blkid.h>

namespace datto_linux_client {

std::unique_ptr<MountableBlockDevice>
BlockDeviceFactory::CreateMountableBlockDevice(std::string path) {
  return nullptr;
}

std::unique_ptr<RemoteBlockDevice>
BlockDeviceFactory::CreateRemoteBlockDevice(std::string hostname,
                                            uint16_t port_num) {
  return nullptr;
}

} // datto_linux_client
