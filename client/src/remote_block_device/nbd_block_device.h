#ifndef DATTO_CLIENT_REMOTE_BLOCK_DEVICE_NBD_BLOCK_DEVICE_H_
#define DATTO_CLIENT_REMOTE_BLOCK_DEVICE_NBD_BLOCK_DEVICE_H_

#include "remote_block_device/remote_block_device.h"

namespace datto_linux_client {

class NbdBlockDevice : RemoteBlockDevice {
 public:
  // local_block_path is the *local* block device (e.g. /dev/nbd0)
  NbdBlockDevice(std::string remote_host, uint16_t remote_port,
                 std::string local_block_path);

  void Connect();
  bool IsConnected();
  void Disconnect();

  // TODO: Should we override (and disallow?) throttling for remote
  // block devices? Depends on the implementation details

  // Parent class calls disconnect
  ~NbdBlockDevice();
 private:
  std::string remote_host_;
};

}

#endif //  DATTO_CLIENT_REMOTE_BLOCK_DEVICE_NBD_BLOCK_DEVICE_H_
