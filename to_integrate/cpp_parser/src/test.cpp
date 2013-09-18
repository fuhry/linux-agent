// Test file
#include "EXTFSPartition.h"
using namespace datto_linux_client;

int main(void) {
  EXTFSPartition extfs;
  std::unique_ptr<const SectorSet> sectors = extfs.GetInUseSectors();
  for(SectorSet::iterator iter = sectors->begin(); iter != sectors->end(); ++iter) {
    std::cout << *iter << std::endl;
  }
  return 0;
}
