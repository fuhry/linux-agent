#include "unsynced_sector_tracker/unsynced_sector_tracker.h"
#include "unsynced_sector_tracker/sector_interval.h"

#include "gtest/gtest.h"

namespace {

using ::datto_linux_client::UnsyncedSectorTracker;
using ::datto_linux_client::SectorInterval;

TEST(UnsyncedSectorTrackerTest, DefaultConstructor) {
  const UnsyncedSectorTracker tracker;

  EXPECT_EQ(0UL, tracker.UnsyncedSectorCount());

  const SectorInterval interval = tracker.GetContinuousUnsyncedSectors();
  EXPECT_EQ(0UL, boost::icl::length(interval));
}

TEST(UnsyncedSectorTrackerTest, AddUnsyncedIntervalTest) {
  UnsyncedSectorTracker tracker;
  SectorInterval interval1(1, 10);
  SectorInterval interval2(5, 20);

  tracker.AddUnsyncedInterval(interval1);
  EXPECT_EQ(9UL, tracker.UnsyncedSectorCount());

  EXPECT_TRUE(interval1 == tracker.GetContinuousUnsyncedSectors());

  tracker.AddUnsyncedInterval(interval2);
  EXPECT_TRUE(SectorInterval(1, 20) == tracker.GetContinuousUnsyncedSectors());
}

TEST(UnsyncedSectorTrackerTest, MarkToSyncIntervalTest) {
  UnsyncedSectorTracker tracker;
  SectorInterval interval1(1, 20);
  SectorInterval interval2(5, 20);

  tracker.AddUnsyncedInterval(interval1);

  tracker.MarkToSyncInterval(interval2);
  EXPECT_TRUE(SectorInterval(1, 5) == tracker.GetContinuousUnsyncedSectors());
  EXPECT_EQ(4UL, tracker.UnsyncedSectorCount());
}

} // namespace
