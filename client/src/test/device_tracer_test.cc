#include "block_trace/device_tracer.h"
#include "block_trace/block_trace_exception.h"

#include <memory>
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

 protected:
  class DummyHandler : public TraceHandler {
   public:
    DummyHandler() : TraceHandler() { }
    virtual void AddTrace(const struct blk_io_trace &trace_data) {

    }
  };

  DeviceTracerTest() {
    CreateEmptyLoopDevice(TEST_BLOCK_DEVICE_SIZE);
    LOG(INFO) << loop_file_name;

    SetLoopFD();
    LOG(INFO) << loop_dev_fd;

    handler = std::shared_ptr<TraceHandler>(new DummyHandler());
  }

  virtual ~DeviceTracerTest() {
    unlink(loop_file_name);
    ClearLoopFD();
  }

  char loop_file_name[L_tmpnam];
  int loop_dev_fd;
  int loop_file_fd;

  std::shared_ptr<TraceHandler> handler;

 private:
  void ClearLoopFD() {
    ioctl(loop_dev_fd, LOOP_CLR_FD);
  }

  void CreateEmptyLoopDevice(uint64_t size_bytes) {
    tmpnam(loop_file_name);

    ASSERT_NE(nullptr, loop_file_name);

    loop_file_fd = creat(loop_file_name, S_IRWXU);

    ASSERT_NE(-1, loop_file_fd) << errno;

    int trun_ret = ftruncate64(loop_file_fd, size_bytes);
    ASSERT_EQ(0, trun_ret) << errno;
  }

  void SetLoopFD() {
    struct loop_info64 loop_info;

    // We don't need to do this on modern Linux's.. see
    // https://lkml.org/lkml/2011/7/26/148
    for (int i = 0; i < 100; ++i) {
      std::string loop_path = "/dev/loop" + std::to_string(i);
      loop_dev_fd = open(loop_path.c_str(), O_RDWR);

      if (loop_dev_fd == -1) {
        PLOG(ERROR) << "Error opening loop device " << i;
        FAIL();
      }

      // If the ioctl succeeds, then the loop device already exists
      if (ioctl(loop_dev_fd, LOOP_GET_STATUS64, &loop_info) >= 0) {
        continue;
      }

      if (ioctl(loop_dev_fd, LOOP_SET_FD, loop_file_fd) < 0) {
        PLOG(ERROR) << "Unable to assign file to loop device";
        FAIL();
      }

      loop_info = {};
      memcpy(loop_info.lo_file_name, loop_file_name,
             strlen(loop_file_name) + 1);

      if (ioctl(loop_dev_fd, LOOP_SET_STATUS64, &loop_info) < 0) {
        PLOG(ERROR) << "Unable to set status for loop device";
        ClearLoopFD();
        FAIL();
      }

      break;
    }
  }

};

TEST_F(DeviceTracerTest, BadConstructor) {
  try {
    DeviceTracer d("/dev/sda", handler);
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
