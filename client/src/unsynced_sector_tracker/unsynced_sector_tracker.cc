#include "unsynced_sector_tracker/unsynced_sector_tracker.h"

namespace datto_linux_client {

UnsyncedSectorTracker::UnsyncedSectorTracker()
    : unsynced_sector_set_(),
      sector_set_mutex_() { }

UnsyncedSectorTracker::~UnsyncedSectorTracker() { }

void UnsyncedSectorTracker::AddUnsyncedInterval(
    const SectorInterval &sector_interval) {
  std::lock_guard<std::mutex> set_lock(sector_set_mutex_);
  unsynced_sector_set_.add(sector_interval);
}

void UnsyncedSectorTracker::MarkToSyncInterval(
    const SectorInterval &sector_interval) {
  std::lock_guard<std::mutex> set_lock(sector_set_mutex_);
  unsynced_sector_set_.subtract(sector_interval);
}

SectorInterval UnsyncedSectorTracker::GetContinuousUnsyncedSectors() const {
  std::lock_guard<std::mutex> set_lock(sector_set_mutex_);

  SectorInterval largestInterval;
  for (auto interval : unsynced_sector_set_) {
    if (boost::icl::length(interval) > boost::icl::length(largestInterval)) {
      largestInterval = interval;
    }
  }

  return largestInterval;
}

uint64_t UnsyncedSectorTracker::UnsyncedSectorCount() const {
  std::lock_guard<std::mutex> set_lock(sector_set_mutex_);

  return boost::icl::cardinality(unsynced_sector_set_);
}

}
