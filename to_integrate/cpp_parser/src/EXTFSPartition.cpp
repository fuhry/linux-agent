#include "EXTFSPartition.h"

namespace datto_linux_client {
  std::unique_ptr<const SectorSet> EXTFSPartition::GetInUseSectors() {
    return NULL;
  }
}
