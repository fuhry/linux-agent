#include "tracing/block_trace_exception.h"
#include "tracing/device_tracer.h"
#include "unsynced_sector_manager/unsynced_sector_store.h"
#include "test/loop_device.h"

#include <memory>
#include <fstream>
#include <string.h>

#include <errno.h>
#include <glog/logging.h>
#include <stdio.h>
#include <unistd.h>

#include <gtest/gtest.h>

namespace {

using ::datto_linux_client::BlockTraceException;
using ::datto_linux_client::DeviceTracer;
using ::datto_linux_client::SectorInterval;
using ::datto_linux_client::TraceHandler;
using ::datto_linux_client::UnsyncedSectorStore;
using ::datto_linux_client_test::LoopDevice;

class DeviceTracerTest : public ::testing::Test {
 protected:
  class DummyHandler : public TraceHandler {
   public:
    DummyHandler() : TraceHandler() { }
    virtual void AddTrace(const struct blk_io_trace &trace_data) { }
  };

  DeviceTracerTest() {
    loop_dev = std::unique_ptr<LoopDevice>(new LoopDevice());
    LOG(INFO) << "Path is: " << loop_dev->path();

    sector_store =
      std::shared_ptr<UnsyncedSectorStore>(new UnsyncedSectorStore(10));
    dummy_handler =
      std::shared_ptr<TraceHandler>(new DummyHandler());
    real_handler =
      std::shared_ptr<TraceHandler>(new TraceHandler(sector_store));
  }

  std::unique_ptr<LoopDevice> loop_dev;

  std::shared_ptr<UnsyncedSectorStore> sector_store;

  std::shared_ptr<TraceHandler> dummy_handler;
  std::shared_ptr<TraceHandler> real_handler;
};

TEST_F(DeviceTracerTest, Constructor) {
  DeviceTracer d(loop_dev->path(), dummy_handler);
}

TEST_F(DeviceTracerTest, ReadWithoutWrites) {
  DeviceTracer d(loop_dev->path(), real_handler);

  std::ifstream loop_in(loop_dev->path());

  loop_in.seekg(0, std::ios::end);
  size_t file_size = loop_in.tellg();
  loop_in.seekg(0, std::ios::beg);

  // Allocate a string the same size as the file
  std::string file_contents(file_size, '\0');

  loop_in.read(&file_contents[0], file_size);
  loop_in.close();

  loop_dev->Sync();
  d.FlushBuffers();

  EXPECT_EQ(0UL, sector_store->UnsyncedSectorCount());
}

TEST_F(DeviceTracerTest, WriteAnything) {
  DeviceTracer d(loop_dev->path(), real_handler);

  std::ofstream loop_out(loop_dev->path());

  loop_out << "Text to trigger a write trace";
  loop_out.close();

  loop_dev->Sync();
  d.FlushBuffers();

  uint64_t unsynced_count = sector_store->UnsyncedSectorCount();
  EXPECT_NE(0UL, unsynced_count);
}

TEST_F(DeviceTracerTest, WriteSpecificLocation) {
  DeviceTracer d(loop_dev->path(), real_handler);

  std::ofstream loop_out(loop_dev->path());

  uint64_t sectors_per_block = loop_dev->block_size() / 512;

  // Write sectors [sectors_per_block * 10, sectors_per_block * 11)
  loop_out.seekp(loop_dev->block_size() * 10, std::ios::beg);
  loop_out << "Text to trigger a write trace";

  // Write sectors [sectors_per_block * 11, sectors_per_block * 12)
  loop_out.seekp(loop_dev->block_size() * 11, std::ios::beg);
  loop_out << "Text to trigger a write trace";

  // Final interval should be
  // [loop_dev_block_size * 10, loop_dev_block_size * 12)

  loop_out.close();

  loop_dev->Sync();
  d.FlushBuffers();

  uint64_t unsynced_count = sector_store->UnsyncedSectorCount();
  auto expected_interval = SectorInterval(sectors_per_block * 10,
      sectors_per_block * 12);

  SectorInterval actual_interval;
  sector_store->GetInterval(&actual_interval, 100);

  EXPECT_EQ(sectors_per_block * 2, unsynced_count);

  EXPECT_EQ(expected_interval.lower(), actual_interval.lower());
  EXPECT_EQ(expected_interval.upper(), actual_interval.upper());
}


} // namespace
