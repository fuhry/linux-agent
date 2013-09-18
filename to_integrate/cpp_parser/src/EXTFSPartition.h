#ifndef DATTO_CLIENT_BLOCK_DEVICE_EXTFSPARTITION_H_
#define DATTO_CLIENT_BLOCK_DEVICE_EXTFSPARTITION_H_

#include "Partition.h"

namespace datto_linux_client {

class EXTFSPartition : private Partition {
 public:
  virtual std::unique_ptr<const SectorSet> GetInUseSectors();
};

}
#endif //  DATTO_CLIENT_BLOCK_DEVICE_EXTFSPARTITION_H_
