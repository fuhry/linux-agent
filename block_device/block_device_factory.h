#ifndef DATTO_CLIENT_BLOCK_DEVICE_BLOCK_DEVICE_FACTORY_H_
#define DATTO_CLIENT_BLOCK_DEVICE_BLOCK_DEVICE_FACTORY_H_

#include <memory>

#include "block_device/block_device.h"
#include "block_device/mountable_block_device.h"
#include "block_device/remote_block_device.h"

namespace datto_linux_client {

class BlockDeviceFactory {
 public:
  BlockDeviceFactory() {}
  virtual ~BlockDeviceFactory() {}

  virtual std::shared_ptr<MountableBlockDevice> CreateMountableBlockDevice(
      std::string path);

  virtual std::shared_ptr<RemoteBlockDevice> CreateRemoteBlockDevice(
      std::string hostname, uint16_t port_num);
};

} // datto_linux_client

#endif //  DATTO_CLIENT_BLOCK_DEVICE_BLOCK_DEVICE_FACTORY_H_
