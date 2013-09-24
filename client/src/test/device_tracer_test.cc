#include "block_trace/device_tracer.h"
#include "block_trace/block_trace_exception.h"

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

using ::datto_linux_client::DeviceTracer;
using ::datto_linux_client::BlockTraceException;
using ::datto_linux_client::TraceHandler;

class DeviceTracerTest : public ::testing::Test {
 public:
  static const uint64_t TEST_BLOCK_DEVICE_SIZE = 1 * 1024 * 1024 * 1024;
  const std::string TEST_LOOP_SHARED_MEMORY = "/dev/shm/test_loop_path";

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

    handler = std::shared_ptr<TraceHandler>(new DummyHandler());
  }

  ~DeviceTracerTest() {
    close(loop_dev_fd);
    system(("losetup -d " + loop_dev_path).c_str());
    unlink(TEST_LOOP_SHARED_MEMORY.c_str());
  }

  std::string loop_dev_path;
  int loop_dev_fd;

  std::shared_ptr<TraceHandler> handler;

 private:
  void CreateEmptyLoopDevice(uint64_t size_bytes) {
    int create_ret = system("./test/make_test_loop_device");
    ASSERT_EQ(0, create_ret);

    std::ifstream loop_path_stream(TEST_LOOP_SHARED_MEMORY);
    std::getline(loop_path_stream, loop_dev_path);
    loop_path_stream.close();

    if ((loop_dev_fd = open(loop_dev_path.c_str(), O_WRONLY)) == -1) {
      PLOG(ERROR) << "Unable to make test loop device. Verify everything is cleaned up with losetup";
      unlink(TEST_LOOP_SHARED_MEMORY.c_str());
      FAIL();
    }
  }
};

TEST_F(DeviceTracerTest, BadConstructor) {
  try {
    DeviceTracer d(loop_dev_path, handler);
    sleep(10);
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
