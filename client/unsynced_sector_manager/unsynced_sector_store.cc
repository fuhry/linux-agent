#include "unsynced_sector_manager/unsynced_sector_store.h"

namespace datto_linux_client {

UnsyncedSectorStore::UnsyncedSectorStore()
    : unsynced_sector_set_(),
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
SectorInterval UnsyncedSectorStore::GetContinuousUnsyncedSectors() const {
  std::lock_guard<std::mutex> set_lock(sector_set_mutex_);

  SectorInterval largestInterval(0, 0);
  for (auto interval : unsynced_sector_set_) {
    if (boost::icl::length(interval) > boost::icl::length(largestInterval)) {
      largestInterval = interval;
    }
  }

  if (boost::icl::length(largestInterval) < 1024
      && boost::icl::length(largestInterval) != 0) {
    return *unsynced_sector_set_.rbegin();
  }

  return largestInterval;
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
