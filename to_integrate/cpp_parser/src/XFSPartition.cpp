#include "XFSPartition.h"
#include <stdbool.h>
#include <error.h>
#include <endian.h>
extern "C" {
#define delete baleet
#include "libxfs/include/libxfs.h"
#undef delete
}
namespace datto_linux_client {
  XFSPartition::XFSPartition() : Partition(NULL) { //TODO: remove this
  }
  
  std::unique_ptr<const SectorSet> XFSPartition::GetInUseSectors() {
    SectorSet *sectors = new SectorSet();
    XFSPartition::xfs_iter_blocks(sectors);    
    return std::unique_ptr<const SectorSet>(sectors);
  }
  
  int XFSPartition::xfs_iter_blocks(SectorSet *sectors) {
    return 0;
  }
}
