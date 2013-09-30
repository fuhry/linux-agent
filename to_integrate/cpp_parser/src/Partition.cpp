#include "Partition.h"

namespace datto_linux_client {
  bool Partition::IsMounted() {
    return false;
  }
  
  std::string Partition::GetMountPoint() {
    return "";
  }

  void Partition::Freeze() {
  }
  
  void Partition::Unfreeze() {
  }

  Partition::~Partition() {
  }

  Partition::Partition(std::unique_ptr<BlockDevice> block_device) {
  }
}
