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
  UnsyncedSectorManager manager("/dev/null");
}

TEST(UnsyncedSectorManagerTest, Tracers) {
  LoopDevice loop_dev;

  {
    UnsyncedSectorManager manager(loop_dev.path());

    manager.StartTracer();
    manager.StopTracer();

    manager.StartTracer();

    try {
      manager.StartTracer();
      FAIL() << "Shouldn't have been able to start another tracer";
    } catch (const std::runtime_error &e) {
      // good
    }
  }

  // Destructor should have been called from previous block
  UnsyncedSectorManager manager(loop_dev.path());
  manager.StartTracer();
}

TEST(UnsyncedSectorManagerTest, store) {
  UnsyncedSectorManager manager("/dev/null");

  {
    std::shared_ptr<UnsyncedSectorStore> store(manager.store());
    store->AddUnsyncedInterval(SectorInterval(0, 10));
  }

  std::shared_ptr<UnsyncedSectorStore> store(manager.store());

  EXPECT_EQ(10UL, store->UnsyncedSectorCount());
}

} // namespace
