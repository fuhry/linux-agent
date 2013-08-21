#include <string>
#include "block_device.h"

namespace datto_linux_client {

std::string BlockDevice::block_path() const {
  return block_path_;
}

BlockDevice::~BlockDevice() { };

BlockDevice::BlockDevice(std::string block_path) {
  block_path_ = block_path;
}

}
