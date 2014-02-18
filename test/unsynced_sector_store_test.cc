#include "unsynced_sector_manager/unsynced_sector_store.h"
#include "unsynced_sector_manager/sector_interval.h"

#include <gtest/gtest.h>

namespace {

using ::datto_linux_client::UnsyncedSectorStore;
using ::datto_linux_client::SectorInterval;

TEST(UnsyncedSectorStoreTest, DefaultConstructor) {
  const UnsyncedSectorStore store;

  EXPECT_EQ(0UL, store.UnsyncedSectorCount());

  const SectorInterval interval = store.GetContinuousUnsyncedSectors();
  EXPECT_EQ(0UL, boost::icl::length(interval));
}

TEST(UnsyncedSectorStoreTest, AddUnsyncedIntervalTest) {
  UnsyncedSectorStore store;
  SectorInterval interval1(1, 10);
  SectorInterval interval2(5, 20);

  store.AddUnsyncedInterval(interval1);
  EXPECT_EQ(9UL, store.UnsyncedSectorCount());

  EXPECT_TRUE(interval1 == store.GetContinuousUnsyncedSectors());

  store.AddUnsyncedInterval(interval2);
  EXPECT_TRUE(SectorInterval(1, 20) == store.GetContinuousUnsyncedSectors());
}

TEST(UnsyncedSectorStoreTest, MarkToSyncIntervalTest) {
  UnsyncedSectorStore store;
  SectorInterval interval1(1, 20);
  SectorInterval interval2(5, 20);

  store.AddUnsyncedInterval(interval1);

  store.MarkToSyncInterval(interval2);
  EXPECT_TRUE(SectorInterval(1, 5) == store.GetContinuousUnsyncedSectors());
  EXPECT_EQ(4UL, store.UnsyncedSectorCount());

  store.MarkToSyncInterval(interval1);
  EXPECT_TRUE(SectorInterval(0, 0) == store.GetContinuousUnsyncedSectors());
  store.ResetUnsynced();
  EXPECT_TRUE(interval1 == store.GetContinuousUnsyncedSectors());

  store.MarkToSyncInterval(interval1);
  EXPECT_TRUE(SectorInterval(0, 0) == store.GetContinuousUnsyncedSectors());
  store.ClearSynced();
  store.ResetUnsynced();
  EXPECT_TRUE(SectorInterval(0, 0) == store.GetContinuousUnsyncedSectors());
}

TEST(UnsyncedSectorStoreTest, ClearAllTest) {
  UnsyncedSectorStore store;
  SectorInterval interval(1, 20);

  store.AddUnsyncedInterval(interval);
  EXPECT_NE(0UL, store.UnsyncedSectorCount());

  store.ClearAll();
  EXPECT_EQ(0UL, store.UnsyncedSectorCount());
}

} // namespace
