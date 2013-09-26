#include "block_trace/block_trace_exception.h"
#include "block_trace/device_tracer.h"
#include "unsynced_sector_tracker/unsynced_sector_tracker.h"

#include <memory>
#include <fstream>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <glog/logging.h>
#include <linux/loop.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "gtest/gtest.h"

namespace {

using ::datto_linux_client::BlockTraceException;
using ::datto_linux_client::DeviceTracer;
using ::datto_linux_client::SectorInterval;
using ::datto_linux_client::TraceHandler;
using ::datto_linux_client::UnsyncedSectorTracker;

// TODO: This class assumes that there is a /dev/shm and /tmp is the
// temporary directory. These assumptions should be made more explicit
// or removed.
static const uint64_t TEST_BLOCK_DEVICE_SIZE = 1 * 1024 * 1024 * 1024;
static const char TEST_LOOP_SHARED_MEMORY[] = "/dev/shm/test_loop_path";

class DeviceTracerTest : public ::testing::Test {
 public:

 protected:
  class DummyHandler : public TraceHandler {
   public:
    DummyHandler() : TraceHandler() { }
    virtual void AddTrace(const struct blk_io_trace &trace_data) {

    }
  };

  DeviceTracerTest() {
    CreateEmptyLoopDevice(TEST_BLOCK_DEVICE_SIZE);
    LOG(INFO) << "Path is: " << loop_dev_path;

    dummy_handler = std::shared_ptr<TraceHandler>(new DummyHandler());

    sector_tracker =
      std::shared_ptr<UnsyncedSectorTracker>(new UnsyncedSectorTracker());
    real_handler =
      std::shared_ptr<TraceHandler>(new TraceHandler(sector_tracker));
  }

  ~DeviceTracerTest() {
    system(("losetup -d " + loop_dev_path).c_str());
    unlink(TEST_LOOP_SHARED_MEMORY);
    system("rm /tmp/test_loop_file.*");
  }

  std::string loop_dev_path;
  size_t loop_dev_block_size;

  std::shared_ptr<TraceHandler> dummy_handler;
  std::shared_ptr<TraceHandler> real_handler;

  std::shared_ptr<UnsyncedSectorTracker> sector_tracker;

 private:
  void CreateEmptyLoopDevice(uint64_t size_bytes) {
    int loop_dev_fd;
    int create_ret = system("./test/make_test_loop_device");
    ASSERT_EQ(0, create_ret);

    std::ifstream loop_path_stream(TEST_LOOP_SHARED_MEMORY);
    std::getline(loop_path_stream, loop_dev_path);
    loop_path_stream.close();

    if ((loop_dev_fd = open(loop_dev_path.c_str(), O_WRONLY)) == -1) {
      PLOG(ERROR) << "Unable to make test loop device."
                  << " Verify everything is cleaned up with losetup";
      unlink(TEST_LOOP_SHARED_MEMORY);
      FAIL();
    }

    ioctl(loop_dev_fd, BLKBSZGET, &loop_dev_block_size);
    if (close(loop_dev_fd)) {
      PLOG(ERROR) << "Error closing loop device";
    }
  }
};

TEST_F(DeviceTracerTest, Constructor) {
  try {
    DeviceTracer d(loop_dev_path, dummy_handler);
  } catch (const std::exception &e) {
    FAIL() << e.what();
  }
}

TEST_F(DeviceTracerTest, ReadWithoutWrites) {
  try {
    DeviceTracer d(loop_dev_path, real_handler);

    std::ifstream loop_in(loop_dev_path);

    loop_in.seekg(0, std::ios::end);
    size_t file_size = loop_in.tellg();
    loop_in.seekg(0, std::ios::beg);

    // Allocate a string the same size as the file
    std::string file_contents(file_size, '\0');

    loop_in.read(&file_contents[0], file_size);
    loop_in.close();

    sync();
    d.FlushBuffers();

    EXPECT_EQ(0UL, sector_tracker->UnsyncedSectorCount());
  } catch (const std::exception &e) {
    FAIL() << e.what();
  }
}

TEST_F(DeviceTracerTest, WriteAnything) {
  try {
    DeviceTracer d(loop_dev_path, real_handler);

    std::ofstream loop_out(loop_dev_path);

    loop_out << "Text to trigger a write trace";
    loop_out.close();

    sync();
    d.FlushBuffers();

    uint64_t unsynced_count = sector_tracker->UnsyncedSectorCount();
    EXPECT_NE(0UL, unsynced_count);
  } catch (const std::exception &e) {
    FAIL() << e.what();
  }
}

TEST_F(DeviceTracerTest, WriteSpecificLocation) {
  try {
    DeviceTracer d(loop_dev_path, real_handler);

    std::ofstream loop_out(loop_dev_path);

    uint64_t sectors_per_block = loop_dev_block_size / 512;

    // Write sectors [sectors_per_block * 10, sectors_per_block * 11)
    loop_out.seekp(loop_dev_block_size * 10, std::ios::beg);
    loop_out << "Text to trigger a write trace";

    // Write sectors [sectors_per_block * 11, sectors_per_block * 12)
    loop_out.seekp(loop_dev_block_size * 11, std::ios::beg);
    loop_out << "Text to trigger a write trace";

    // Final interval should be
    // [loop_dev_block_size * 10, loop_dev_block_size * 12)

    loop_out.close();

    sync();
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
// Set up fake block device
// Start tracing that device
// Write to known locations
// Verify that the TraceHandler got traces for those locations
// Take down trace
// Write to known locations
// Verify the TraceHandler did not get traces for those locations


} // namespace
