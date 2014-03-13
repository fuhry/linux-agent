#include "unsynced_sector_manager/unsynced_sector_store.h"
#include "unsynced_sector_manager/sector_interval.h"

#include <gtest/gtest.h>

namespace {

using ::datto_linux_client::UnsyncedSectorStore;
using ::datto_linux_client::SectorInterval;

// Basic tests

TEST(UnsyncedSectorStoreTest, DefaultConstructor) {
  UnsyncedSectorStore store;

  EXPECT_EQ(0UL, store.UnsyncedSectorCount());

  SectorInterval output_interval;

  store.GetInterval(&output_interval, 11);
  EXPECT_EQ(0UL, boost::icl::length(output_interval));
}

TEST(UnsyncedSectorStoreTest, AddIntervalTest) {
  UnsyncedSectorStore store;
  SectorInterval interval1(1, 10);
  SectorInterval interval2(5, 20);

  SectorInterval output_interval;

  store.AddInterval(interval1, 10);
  EXPECT_EQ(9UL, store.UnsyncedSectorCount());

  store.GetInterval(&output_interval, 11);
  EXPECT_TRUE(interval1 == output_interval) << output_interval;

  store.AddInterval(interval2, 10);
  store.GetInterval(&output_interval, 11);
  EXPECT_TRUE(SectorInterval(1, 20) == output_interval) << output_interval;
}

TEST(UnsyncedSectorStoreTest, MarkToSyncIntervalTest) {
  UnsyncedSectorStore store;
  SectorInterval interval1(1, 20);
  SectorInterval interval2(5, 20);
  SectorInterval output_interval;

  store.AddInterval(interval1, 11);
  store.RemoveInterval(interval2);
  store.GetInterval(&output_interval, 11);

  EXPECT_TRUE(SectorInterval(1, 5) == output_interval);
  EXPECT_EQ(4UL, store.UnsyncedSectorCount());

  store.RemoveInterval(interval1);
  store.GetInterval(&output_interval, 11);
  EXPECT_TRUE(SectorInterval(0, 0) == output_interval) << output_interval;
}

TEST(UnsyncedSectorStoreTest, ClearAllTest) {
  UnsyncedSectorStore store;
  SectorInterval interval(1, 20);

  store.AddInterval(interval, 11);
  EXPECT_NE(0UL, store.UnsyncedSectorCount());

  store.ClearIntervals();
  EXPECT_EQ(0UL, store.UnsyncedSectorCount());
}

// Timing tests

} // namespace
