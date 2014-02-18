#include "unsynced_sector_manager/unsynced_sector_manager.h"

#include <memory>

#include <glog/logging.h>
#include <gtest/gtest.h>

#include "unsynced_sector_manager/unsynced_sector_store.h"
#include "unsynced_sector_manager/sector_interval.h"
#include "test/loop_device.h"

namespace {

using ::datto_linux_client::BlockDevice;
using ::datto_linux_client::DeviceTracer;
using ::datto_linux_client::UnsyncedSectorManager;
using ::datto_linux_client::UnsyncedSectorStore;
using ::datto_linux_client::SectorInterval;
using ::datto_linux_client_test::LoopDevice;

class StubDeviceTracer : public DeviceTracer {
 public:
  StubDeviceTracer() {}
  ~StubDeviceTracer() {}
  virtual void CleanupBlockTrace() {}
};

class TestableUnsyncedSectorManager : public UnsyncedSectorManager {
 public:
  TestableUnsyncedSectorManager() {}
  ~TestableUnsyncedSectorManager() {}

  virtual std::unique_ptr<DeviceTracer> CreateDeviceTracer(
      std::string path, std::shared_ptr<UnsyncedSectorStore> store) {
    auto tracer = std::unique_ptr<DeviceTracer>(new StubDeviceTracer());
    return tracer;
  }
};

TEST(UnsyncedSectorManagerTest, Constructor) {
  TestableUnsyncedSectorManager manager;
}

TEST(UnsyncedSectorManagerTest, Tracers) {
  LoopDevice loop_dev;
  BlockDevice loop_block(loop_dev.path());

  {
    TestableUnsyncedSectorManager manager;

    manager.StartTracer(loop_block);
    manager.StopTracer(loop_block);

    manager.StartTracer(loop_block);

    try {
      manager.StartTracer(loop_block);
      FAIL() << "Shouldn't have been able to start another tracer";
    } catch (const std::runtime_error &e) {
      // good
    }
  }

  // Destructor should have been called from previous block
  TestableUnsyncedSectorManager manager;
  manager.StartTracer(loop_block);
}

TEST(UnsyncedSectorManagerTest, store) {
  TestableUnsyncedSectorManager manager;

  LoopDevice loop_dev;
  BlockDevice loop_block(loop_dev.path());

  {
    std::shared_ptr<UnsyncedSectorStore> store(manager.GetStore(loop_block));
    EXPECT_FALSE(store == nullptr);
    store->AddUnsyncedInterval(SectorInterval(0, 10));
  }

  std::shared_ptr<UnsyncedSectorStore> store(manager.GetStore(loop_block));

  EXPECT_EQ(10UL, store->UnsyncedSectorCount());
}

} // namespace
