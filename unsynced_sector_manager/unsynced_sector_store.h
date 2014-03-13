#ifndef DATTO_CLIENT_UNSYNCED_SECTOR_MANAGER_UNSYNCED_SECTOR_STORE_H_
#define DATTO_CLIENT_UNSYNCED_SECTOR_MANAGER_UNSYNCED_SECTOR_STORE_H_

#include "unsynced_sector_manager/sector_interval.h"
#include "unsynced_sector_manager/sector_set.h"
#include "unsynced_sector_manager/timed_sector_map.h"

#include <mutex>
#include <stdint.h>
#include <time.h>

namespace datto_linux_client {

class UnsyncedSectorStore {
 public:
  // volatile_seconds is the number of seconds ago a write should have
  // occurred to not require special handling (i.e. a filesystem freeze)
  explicit UnsyncedSectorStore(int volatile_seconds);
  virtual ~UnsyncedSectorStore() {}

  // Add an interval that has not been modified recently
  virtual void AddNonVolatileInterval(const SectorInterval &sector_interval);

  // Add an interval that was modified recently. This is intended for inserting
  // block trace data
  virtual void AddInterval(const SectorInterval &sector_interval,
                           const time_t time);

  // Copies an unsynced interval into output.
  // epoch is the current time (as returned by time())
  //
  // The return value indicated if the interval was modified in the
  // past volatile_seconds and thus should need a file system freeze.
  virtual bool GetInterval(SectorInterval *const output,
                           const time_t epoch) const;

  // Removes the marked interval
  // This should be called before copying an interval to the destination
  virtual void RemoveInterval(const SectorInterval &sector_interval);

  // Clears the entire Store
  virtual void ClearIntervals();

  // Clears synced intervals. Should be called when a backup completes
  virtual void ClearSyncHistory();

  // Loads the synced intervals into the unsynced intervals
  // This should be called when a backup is stopped or fails
  virtual void ReInsertSyncHistory();

  // Returns the total number of unsynced sectors
  virtual uint64_t UnsyncedSectorCount() const;
 private:
  const int volatile_seconds_;
  TimedSectorMap unsynced_sector_map_;
  SectorSet synced_sector_set_;
  mutable uint64_t end_of_last_continuous_;
  mutable std::mutex mutex_;
};

}

#endif //  DATTO_CLIENT_UNSYNCED_SECTOR_MANAGER_UNSYNCED_SECTOR_STORE_H_
