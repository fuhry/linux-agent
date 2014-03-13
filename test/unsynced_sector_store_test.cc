#include "unsynced_sector_manager/unsynced_sector_store.h"
#include "unsynced_sector_manager/sector_interval.h"

#include <gtest/gtest.h>

namespace {

using ::datto_linux_client::UnsyncedSectorStore;
using ::datto_linux_client::SectorInterval;

// Basic tests

TEST(UnsyncedSectorStoreTest, DefaultConstructor) {
  UnsyncedSectorStore store(10);

  EXPECT_EQ(0UL, store.UnsyncedSectorCount());

  SectorInterval output_interval;

  store.GetInterval(&output_interval, time(NULL));
  EXPECT_EQ(0UL, boost::icl::length(output_interval));
}

TEST(UnsyncedSectorStoreTest, AddIntervalTest) {
  UnsyncedSectorStore store(10);
  SectorInterval interval1(1, 10);
  SectorInterval interval2(5, 20);

  SectorInterval output_interval;

  store.AddInterval(interval1, 1000);
  EXPECT_EQ(9UL, store.UnsyncedSectorCount());

  store.GetInterval(&output_interval, time(NULL));
  EXPECT_TRUE(interval1 == output_interval) << output_interval;

  store.AddInterval(interval2, 1000);
  store.GetInterval(&output_interval, time(NULL));
  EXPECT_TRUE(SectorInterval(1, 20) == output_interval) << output_interval;
}

TEST(UnsyncedSectorStoreTest, RemoveIntervalTest) {
  UnsyncedSectorStore store(10);
  SectorInterval interval1(1, 20);
  SectorInterval interval2(5, 20);
  SectorInterval output_interval;

  store.AddInterval(interval1, 1000);
  store.RemoveInterval(interval2);
  store.GetInterval(&output_interval, time(NULL));

  EXPECT_TRUE(SectorInterval(1, 5) == output_interval) << output_interval;
  EXPECT_EQ(4UL, store.UnsyncedSectorCount());

  store.RemoveInterval(interval1);
  store.GetInterval(&output_interval, time(NULL));
  EXPECT_TRUE(SectorInterval(0, 0) == output_interval) << output_interval;
}

TEST(UnsyncedSectorStoreTest, ClearAllTest) {
  UnsyncedSectorStore store(10);
  SectorInterval interval(1, 20);

  store.AddInterval(interval, 1000);
  EXPECT_NE(0UL, store.UnsyncedSectorCount());

  store.ClearIntervals();
  EXPECT_EQ(0UL, store.UnsyncedSectorCount());
}

// Timing tests

TEST(UnsyncedSectorStoreTest, VolatileTest) {
  UnsyncedSectorStore store(10);
  SectorInterval interval1(1, 20);
  SectorInterval output_interval;

  store.AddInterval(interval1, 1000);
  bool is_volatile = store.GetInterval(&output_interval, 1005);
  EXPECT_TRUE(is_volatile);

  is_volatile = store.GetInterval(&output_interval, 2000);
  EXPECT_FALSE(is_volatile);
}

TEST(UnsyncedSectorStoreTest, AddNonVolatileTest) {
  UnsyncedSectorStore store(10);
  SectorInterval interval1(1, 20);
  SectorInterval output_interval;

  store.AddNonVolatileInterval(interval1);
  bool is_volatile = store.GetInterval(&output_interval, 1005);
  EXPECT_FALSE(is_volatile);
}

} // namespace
