#include "block_trace/block_trace_exception.h"
#include "block_trace/device_tracer.h"
#include "unsynced_sector_tracker/unsynced_sector_tracker.h"
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
using ::datto_linux_client::UnsyncedSectorTracker;
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

    sector_tracker =
      std::shared_ptr<UnsyncedSectorTracker>(new UnsyncedSectorTracker());
    dummy_handler =
      std::shared_ptr<TraceHandler>(new DummyHandler());
    real_handler =
      std::shared_ptr<TraceHandler>(new TraceHandler(sector_tracker));
  }

  std::unique_ptr<LoopDevice> loop_dev;

  std::shared_ptr<UnsyncedSectorTracker> sector_tracker;

  std::shared_ptr<TraceHandler> dummy_handler;
  std::shared_ptr<TraceHandler> real_handler;
};

TEST_F(DeviceTracerTest, Constructor) {
  try {
    DeviceTracer d(loop_dev->path(), dummy_handler);
  } catch (const std::exception &e) {
    FAIL() << e.what();
  }
}

TEST_F(DeviceTracerTest, ReadWithoutWrites) {
  try {
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

    EXPECT_EQ(0UL, sector_tracker->UnsyncedSectorCount());
  } catch (const std::exception &e) {
    FAIL() << e.what();
  }
}

TEST_F(DeviceTracerTest, WriteAnything) {
  try {
    DeviceTracer d(loop_dev->path(), real_handler);

    std::ofstream loop_out(loop_dev->path());

    loop_out << "Text to trigger a write trace";
    loop_out.close();

    loop_dev->Sync();
    d.FlushBuffers();

    uint64_t unsynced_count = sector_tracker->UnsyncedSectorCount();
    EXPECT_NE(0UL, unsynced_count);
  } catch (const std::exception &e) {
    FAIL() << e.what();
  }
}

TEST_F(DeviceTracerTest, WriteSpecificLocation) {
  try {
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

    uint64_t unsynced_count = sector_tracker->UnsyncedSectorCount();
    auto expected_interval = SectorInterval(sectors_per_block * 10,
                                            sectors_per_block * 12);

    SectorInterval continuous_interval =
        sector_tracker->GetContinuousUnsyncedSectors();

    EXPECT_EQ(sectors_per_block * 2, unsynced_count);

    EXPECT_EQ(expected_interval.lower(), continuous_interval.lower());
    EXPECT_EQ(expected_interval.upper(), continuous_interval.upper());

  } catch (const std::exception &e) {
    FAIL() << e.what();
  }
}


} // namespace
