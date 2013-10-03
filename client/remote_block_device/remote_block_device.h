#ifndef DATTO_CLIENT_REMOTE_BLOCK_DEVICE_REMOTE_BLOCK_DEVICE_H_
#define DATTO_CLIENT_REMOTE_BLOCK_DEVICE_REMOTE_BLOCK_DEVICE_H_

#include "block_device/block_device.h"
#include <boost/noncopyable.hpp>

namespace datto_linux_client {

class RemoteBlockDevice : public BlockDevice {
 public:
  virtual bool IsConnected() const = 0;
  // Disconnect should not return an exception if already disconnected
  virtual void Disconnect() = 0;

  virtual ~RemoteBlockDevice() { }
 protected:
  RemoteBlockDevice() : BlockDevice() { } 
};

}

#endif //  DATTO_CLIENT_REMOTE_BLOCK_DEVICE_REMOTE_BLOCK_DEVICE_H_
