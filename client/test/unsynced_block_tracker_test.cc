#include "unsynced_block_tracker/unsynced_block_tracker.h"
#include "unsynced_block_tracker/sector_interval.h"

#include <stdio.h>

int main() {
  datto_linux_client::UnsyncedSectorTracker subject;

  subject.AddUnsyncedSector(123);
  subject.AddUnsyncedSector(124);
  subject.AddUnsyncedSector(123);

  subject.AddUnsyncedSector(125);

  subject.AddUnsyncedInterval(
      datto_linux_client::SectorInterval(200, 300));

  printf("%ld\n", subject.NumberUnsynced());

  printf("%ld\n", boost::icl::length(subject.GetContinuousUnsyncedSectors()));

}
