#include "device_synchronizer/device_synchronizer.h"
#include "block_device/block_device.h"
#include "request_listener/reply_channel.h"
#include "block_device/mountable_block_device.h"
#include "unsynced_sector_tracker/unsynced_sector_tracker.h"
#include "unsynced_sector_tracker/sector_interval.h"
#include "unsynced_sector_tracker/sector_set.h"
#include "test/loop_device.h"

#include <memory>

#include <glog/logging.h>
#include <unistd.h>

#include <gtest/gtest.h>

namespace {

using ::datto_linux_client::DeviceSynchronizer;
using ::datto_linux_client::MountableBlockDevice;
using ::datto_linux_client::Reply;
using ::datto_linux_client::ReplyChannel;
using ::datto_linux_client::SectorInterval;
using ::datto_linux_client::SectorSet;
using ::datto_linux_client::UnsyncedSectorTracker;
using ::datto_linux_client_test::LoopDevice;

class DummyMountableBlockDevice : public MountableBlockDevice {
 public:
  explicit DummyMountableBlockDevice(std::string b_block_path)
      : MountableBlockDevice(b_block_path) { }

  std::unique_ptr<const SectorSet> GetInUseSectors() {
    return nullptr;
  }
};

class DummyReplyChannel : public ReplyChannel {
 public:
  DummyReplyChannel() : is_available_(true) { }
  virtual void SendReply(std::shared_ptr<Reply> reply) { }
  virtual bool IsAvailable() {
    return is_available_;
  }
  void set_is_available(bool a_is_available) {
    is_available_ = a_is_available;
  }

  virtual ~DummyReplyChannel() { }

 private:
  bool is_available_;
};

class DeviceSynchronizerTest : public ::testing::Test {
 protected:
  DeviceSynchronizerTest() {
    source_loop = std::make_shared<LoopDevice>();
    source_loop->FormatAsExt3();
    destination_loop = std::make_shared<LoopDevice>();
    destination_loop->FormatAsExt3();

    source_device = std::make_shared<DummyMountableBlockDevice>(
                        source_loop->path());

    destination_device = std::make_shared<DummyMountableBlockDevice>(
                            destination_loop->path());

    sector_tracker = std::make_shared<UnsyncedSectorTracker>();

    reply_channel = std::make_shared<DummyReplyChannel>();
  }

  void ConstructSynchronizer() {
    device_synchronizer = std::make_shared<DeviceSynchronizer>(
                              source_device,
                              sector_tracker,
                              destination_device,
                              reply_channel);
  }

  ~DeviceSynchronizerTest() { }

  // Order matters here, things will be destructed in opposite order
  // of declaration
  std::shared_ptr<UnsyncedSectorTracker> sector_tracker;

  std::shared_ptr<LoopDevice> source_loop;
  std::shared_ptr<MountableBlockDevice> source_device;

  std::shared_ptr<LoopDevice> destination_loop;
  std::shared_ptr<MountableBlockDevice> destination_device;

  std::shared_ptr<DummyReplyChannel> reply_channel;

  std::shared_ptr<DeviceSynchronizer> device_synchronizer;
};

}

TEST_F(DeviceSynchronizerTest, CanConstruct) {
  // Make sure there is something to sync
  sector_tracker->AddUnsyncedInterval(SectorInterval(0, 1));
  ConstructSynchronizer();
  EXPECT_NE(nullptr, device_synchronizer);
}
