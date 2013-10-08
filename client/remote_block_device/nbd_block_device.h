#ifndef DATTO_CLIENT_REMOTE_BLOCK_DEVICE_NBD_BLOCK_DEVICE_H_
#define DATTO_CLIENT_REMOTE_BLOCK_DEVICE_NBD_BLOCK_DEVICE_H_

#include <string>
#include <memory>
#include <stdint.h>
#include <unistd.h>

#include "remote_block_device/remote_block_device.h"
#include "remote_block_device/nbd_client.h"

namespace datto_linux_client {

class NbdBlockDevice : public RemoteBlockDevice {
 public:
  // local_block_path is the *local* block device (e.g. /dev/nbd0)
  NbdBlockDevice(std::string remote_host, uint16_t remote_port);

  bool IsConnected() const;
  void Disconnect();

  // TODO: Should we override (and disallow?) throttling for remote
  // block devices? Depends on the implementation details

  // Parent class calls disconnect
  ~NbdBlockDevice();
 private:
  std::unique_ptr<NbdClient> nbd_client_;
};

}

#endif //  DATTO_CLIENT_REMOTE_BLOCK_DEVICE_NBD_BLOCK_DEVICE_H_
