#ifndef DATTO_CLIENT_BLOCK_DEVICE_XFSPARTITION_H_
#define DATTO_CLIENT_BLOCK_DEVICE_XFSPARTITION_H_

#include "Partition.h"

namespace datto_linux_client {

class XFSPartition : public Partition {
 public:
  XFSPartition();
  virtual std::unique_ptr<const SectorSet> GetInUseSectors();
 private:
  int xfs_iter_blocks(SectorSet *sectors);
};

}
#endif //  DATTO_CLIENT_BLOCK_DEVICE_XFSPARTITION_H_
