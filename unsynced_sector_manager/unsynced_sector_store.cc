#include "unsynced_sector_manager/unsynced_sector_store.h"
#include <glog/logging.h>

#include <algorithm>

namespace datto_linux_client {

UnsyncedSectorStore::UnsyncedSectorStore(int volatile_seconds)
    : volatile_seconds_(volatile_seconds),
      unsynced_sector_map_(),
      synced_sector_set_(),
      end_of_last_continuous_(0),
      mutex_() { }

void UnsyncedSectorStore::AddInterval(const SectorInterval &sector_interval,
                                      const time_t epoch) {
  std::lock_guard<std::mutex> set_lock(mutex_);
  // For some reason, when epoch = 0 the sector_interval doesn't get
  // inserted. As epoch should never be less volatile_seconds anyway,
  // assert it.
  CHECK_GT(epoch, volatile_seconds_);
  unsynced_sector_map_ += std::make_pair(sector_interval, epoch);
}

void UnsyncedSectorStore::AddNonVolatileInterval(
    const SectorInterval &sector_interval) {
  std::lock_guard<std::mutex> set_lock(mutex_);
  unsynced_sector_map_ += std::make_pair(sector_interval, (time_t)1);
}

void UnsyncedSectorStore::RemoveInterval(
    const SectorInterval &sector_interval) {
  std::lock_guard<std::mutex> set_lock(mutex_);
  unsynced_sector_map_ -= sector_interval;
  synced_sector_set_.add(sector_interval);
}

// Return the intervals sequentially
// TODO: Should this logic be here?
bool UnsyncedSectorStore::GetInterval(SectorInterval *const output,
                                      const time_t epoch) const {
  std::lock_guard<std::mutex> set_lock(mutex_);
  CHECK_GT(epoch, volatile_seconds_);

  bool found_interval = false;
  bool is_volatile = false;

  for (auto interval_pair : unsynced_sector_map_) {
    if (interval_pair.first.lower() > end_of_last_continuous_) {
      // Return the interval directly after the last interval returned
      *output = interval_pair.first;
      VLOG(2) << *output;
      found_interval = true;
      is_volatile = interval_pair.second > (epoch - volatile_seconds_);
      break;
    }
  }

  if (!found_interval) {
    if (unsynced_sector_map_.size() > 0) {
      VLOG(2) << "no output: " << *output;
      auto first_interval_pair = *unsynced_sector_map_.begin();
      *output = first_interval_pair.first;
      is_volatile = first_interval_pair.second > (epoch - volatile_seconds_);
    } else {
      *output = SectorInterval(0, 0);
    }
  }

  end_of_last_continuous_ = output->upper();
  return is_volatile;
}

void UnsyncedSectorStore::ClearIntervals() {
  std::lock_guard<std::mutex> set_lock(mutex_);
  unsynced_sector_map_ = TimedSectorMap();
  synced_sector_set_ = SectorSet();
}

void UnsyncedSectorStore::ClearSyncHistory() {
  std::lock_guard<std::mutex> set_lock(mutex_);
  synced_sector_set_ = SectorSet();
}

void UnsyncedSectorStore::ReInsertSyncHistory() {
  std::lock_guard<std::mutex> set_lock(mutex_);
  for (auto interval : synced_sector_set_) {
    unsynced_sector_map_ += std::make_pair(interval, (time_t)0);
  }
  synced_sector_set_ = SectorSet();
}

uint64_t UnsyncedSectorStore::UnsyncedSectorCount() const {
  std::lock_guard<std::mutex> set_lock(mutex_);
  return boost::icl::cardinality(unsynced_sector_map_);
}

}
