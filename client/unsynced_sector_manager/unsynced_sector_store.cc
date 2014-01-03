#include "unsynced_sector_manager/unsynced_sector_store.h"

#include <algorithm>

namespace datto_linux_client {

UnsyncedSectorStore::UnsyncedSectorStore()
    : unsynced_sector_set_(),
      synced_sector_set_(),
      start_of_last_continuous_(0),
      sector_set_mutex_() { }

UnsyncedSectorStore::~UnsyncedSectorStore() { }

void UnsyncedSectorStore::AddUnsyncedInterval(
    const SectorInterval &sector_interval) {
  std::lock_guard<std::mutex> set_lock(sector_set_mutex_);
  unsynced_sector_set_.add(sector_interval);
}

void UnsyncedSectorStore::MarkToSyncInterval(
    const SectorInterval &sector_interval) {
  std::lock_guard<std::mutex> set_lock(sector_set_mutex_);
  unsynced_sector_set_.subtract(sector_interval);
  synced_sector_set_.add(sector_interval);
}

// For now, we just return the largest unsynced interval. If performance
// becomes an issue, revisit this. Sequential might be quicker
// TODO: Should this logic be here?
SectorInterval UnsyncedSectorStore::GetContinuousUnsyncedSectors() const {
  std::lock_guard<std::mutex> set_lock(sector_set_mutex_);

  // Return the largest interval greater than 1024 if it exists,
  // otherwise return the one directly after the last returned interval
  // to try to take advantage of read-ahead
  SectorInterval interval_to_return(0, 0);
  bool found_big_or_after = false;

  for (auto interval : unsynced_sector_set_) {
    if (interval_to_return.upper() == 0) {
      // This way we always return something
      interval_to_return = interval;
    } else if (boost::icl::length(interval) > 
               std::max(1024ULL, boost::icl::length(interval_to_return))) {
      // Return the largest interval larger than 1024 if it exists
      interval_to_return = interval;
      found_big_or_after = true;
    } else if (!found_big_or_after
               && interval.lower() > start_of_last_continuous_) {
      // Return the one directly after the last one, making sure
      // that we already haven't found a large interval (or found the one
      // directly after already)
      interval_to_return = interval;
      found_big_or_after = true;
    }
  }

  start_of_last_continuous_ = interval_to_return.lower();
  return interval_to_return;
}

void UnsyncedSectorStore::ClearAll() {
  std::lock_guard<std::mutex> set_lock(sector_set_mutex_);
  unsynced_sector_set_ = SectorSet();
  synced_sector_set_ = SectorSet();
}

void UnsyncedSectorStore::ClearSynced() {
  std::lock_guard<std::mutex> set_lock(sector_set_mutex_);
  synced_sector_set_ = SectorSet();
}

void UnsyncedSectorStore::ResetUnsynced() {
  std::lock_guard<std::mutex> set_lock(sector_set_mutex_);
  unsynced_sector_set_ += synced_sector_set_;
}

uint64_t UnsyncedSectorStore::UnsyncedSectorCount() const {
  std::lock_guard<std::mutex> set_lock(sector_set_mutex_);

  return boost::icl::cardinality(unsynced_sector_set_);
}

}
