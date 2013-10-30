#ifndef DATTO_CLIENT_UNSYNCED_SECTOR_STORE_UNSYNCED_SECTOR_STORE_H_
#define DATTO_CLIENT_UNSYNCED_SECTOR_STORE_UNSYNCED_SECTOR_STORE_H_

#include "unsynced_sector_manager/sector_interval.h"
#include "unsynced_sector_manager/sector_set.h"

#include <mutex>
#include <stdint.h>

namespace datto_linux_client {

class UnsyncedSectorStore {
 public:
  UnsyncedSectorStore();
  ~UnsyncedSectorStore();

  void AddUnsyncedInterval(const SectorInterval &sector_interval);

  // This must be called *before* syncing to prevent the situation where
  // the following order of events occurs:
  // 1. Sync the sector
  // 2. *Sector is modified*
  // 3. Mark the, now outdated sector, as synced
  void MarkToSyncInterval(const SectorInterval &sector_interval);

  // Clears the entire Store
  void Clear();

  SectorInterval GetContinuousUnsyncedSectors() const;

  uint64_t UnsyncedSectorCount() const;
 private:
  SectorSet unsynced_sector_set_;
  mutable std::mutex sector_set_mutex_ ;
};

}

#endif //  DATTO_CLIENT_UNSYNCED_SECTOR_STORE_UNSYNCED_SECTOR_STORE_H_
