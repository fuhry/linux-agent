// Test file
#include "EXTFSPartition.h"
#include "XFSPartition.h"
using namespace datto_linux_client;

int main(void) {
  EXTFSPartition extfs;
  XFSPartition xfs;
  std::unique_ptr<const SectorSet> sectors;
  
  sectors = extfs.GetInUseSectors();
  for(SectorSet::iterator iter = sectors->begin(); iter != sectors->end(); ++iter) {
    std::cout << *iter << std::endl;
  }
  
  sectors = xfs.GetInUseSectors();
  for(SectorSet::iterator iter = sectors->begin(); iter != sectors->end(); ++iter) {
    std::cout << *iter << std::endl;
  }
  return 0;
}
