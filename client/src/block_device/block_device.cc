#include "block_device/block_device.h"

namespace datto_linux_client {

BlockDevice::BlockDevice(std::string block_path) : block_path_(block_path) {
  // TODO: Verify block_path exists
  // TODO: set major/minor
}

BlockDevice::BlockDevice(uint32_t major, uint32_t minor)
    : major_(major), minor_(minor) {
  // TODO: Verify major/minor exists
  // TODO: mount block device.. use mknod, makedev
}

std::string BlockDevice::block_path() const {
  return block_path_;
}

uint32_t BlockDevice::major() const {
  return major_;
}

uint32_t BlockDevice::minor() const {
  return minor_;
}

uint64_t BlockDevice::SizeInBytes() const {
  // TODO
  throw "not implemented";
}

BlockDevice::~BlockDevice() {
  // TODO: unmount block device if we mounted it
};

}
