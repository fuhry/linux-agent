#include "unsynced_sector_manager/unsynced_sector_manager.h"

#include <memory>

#include <glog/logging.h>
#include <gtest/gtest.h>

#include "unsynced_sector_manager/unsynced_sector_store.h"
#include "unsynced_sector_manager/sector_interval.h"
#include "test/loop_device.h"

namespace {

using ::datto_linux_client::UnsyncedSectorManager;
using ::datto_linux_client::UnsyncedSectorStore;
using ::datto_linux_client::SectorInterval;
using ::datto_linux_client_test::LoopDevice;

TEST(UnsyncedSectorManagerTest, Constructor) {
  UnsyncedSectorManager manager();
}

TEST(UnsyncedSectorManagerTest, Tracers) {
  LoopDevice loop_dev;

  {
    UnsyncedSectorManager manager;
    // Should be fine if nothing is being traced
    manager.StopAllTracers();

    manager.StartTracer(loop_dev.path());
    manager.StopTracer(loop_dev.path());

    manager.StartTracer(loop_dev.path());
    manager.StopAllTracers();

    manager.StartTracer(loop_dev.path());

    try {
      manager.StartTracer(loop_dev.path());
      FAIL() << "Shouldn't have been able to start another tracer";
    } catch (const std::runtime_error &e) {
      // good
    }
  }

  // Destructor should have been called from previous block
  UnsyncedSectorManager manager;
  manager.StartTracer(loop_dev.path());
}

TEST(UnsyncedSectorManagerTest, GetStore) {
  UnsyncedSectorManager manager;

  {
    std::shared_ptr<UnsyncedSectorStore> store(manager.GetStore("/dev/null"));
    store->AddUnsyncedInterval(SectorInterval(0, 10));
  }

  std::shared_ptr<UnsyncedSectorStore> store(manager.GetStore("/dev/null"));

  EXPECT_EQ(10UL, store->UnsyncedSectorCount());
}

} // namespace
