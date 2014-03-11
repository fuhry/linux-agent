#include "unsynced_sector_manager/unsynced_sector_store.h"

#include <algorithm>

namespace datto_linux_client {

UnsyncedSectorStore::UnsyncedSectorStore()
    : unsynced_sector_set_(),
      synced_sector_set_(),
      end_of_last_continuous_(0),
      sector_set_mutex_() { }

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

// Return the intervals sequentially
// TODO: Should this logic be here?
SectorInterval UnsyncedSectorStore::GetContinuousUnsyncedSectors() const {
  std::lock_guard<std::mutex> set_lock(sector_set_mutex_);

  SectorInterval interval_to_return(0, 0);

  bool found_interval = false;

  for (auto interval : unsynced_sector_set_) {
    if (interval.lower() > end_of_last_continuous_) {
      // Return the interval directory after the last interval
      interval_to_return = interval;
      found_interval = true;
      break;
    }
  }

  if (!found_interval && unsynced_sector_set_.size() > 0) {
    interval_to_return = *unsynced_sector_set_.begin();
  }

  end_of_last_continuous_ = interval_to_return.upper();
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
