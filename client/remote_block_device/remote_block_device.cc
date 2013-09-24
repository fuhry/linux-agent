#include "remote_block_device/remote_block_device.h"

namespace datto_linux_client {

RemoteBlockDevice::~RemoteBlockDevice() {
  try {
    Disconnect();
  } catch (...) {
    // TODO: Log
  }
}

}
