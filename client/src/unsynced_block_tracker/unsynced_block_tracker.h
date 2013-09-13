#ifndef DATTO_CLIENT_UNSYNCED_BLOCK_TRACKER_UNSYNCED_BLOCK_TRACKER_H_
#define DATTO_CLIENT_UNSYNCED_BLOCK_TRACKER_UNSYNCED_BLOCK_TRACKER_H_

#include <boost/icl/interval_set.hpp>
#include "unsynced_block_tracker/sector_interval.h"
#include <stdint.h>
#include <mutex>

namespace datto_linux_client {

class UnsyncedSectorTracker {
 public:
  UnsyncedSectorTracker();
  ~UnsyncedSectorTracker();

  void AddUnsyncedSector(uint64_t sector);
  void AddUnsyncedInterval(const SectorInterval &sector_interval);

  // These must be called *before* syncing to prevent the situation where
  // the following order of events occurs:
  // 1. Sync the sector
  // 2. *Sector is modified*
  // 3. Mark the, now outdated sector, as synced
  void MarkToSyncSector(uint64_t sector);
  void MarkToSyncInterval(const SectorInterval &sector_interval);

  SectorInterval GetContinuousUnsyncedSectors() const;

  uint64_t NumberUnsynced() const;
 private:
  boost::icl::interval_set<uint64_t> unsynced_sector_set_;
  mutable std::mutex sector_set_mutex_ ;
};

}

#endif //  DATTO_CLIENT_UNSYNCED_BLOCK_TRACKER_UNSYNCED_BLOCK_TRACKER_H_
