#include "device_synchronizer/device_synchronizer.h"
#include "block_device/block_device.h"
#include "request_listener/reply_channel.h"
#include "block_device/mountable_block_device.h"
#include "unsynced_sector_manager/sector_interval.h"
#include "unsynced_sector_manager/sector_set.h"
#include "test/loop_device.h"

#include <memory>
#include <array>

#include <glog/logging.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <gtest/gtest.h>

namespace {

using ::datto_linux_client::DeviceSynchronizer;
using ::datto_linux_client::MountableBlockDevice;
using ::datto_linux_client::Reply;
using ::datto_linux_client::ReplyChannel;
using ::datto_linux_client::SectorInterval;
using ::datto_linux_client::SectorSet;
using ::datto_linux_client::UnsyncedSectorManager;
using ::datto_linux_client_test::LoopDevice;

class DummyMountableBlockDevice : public MountableBlockDevice {
 public:
  explicit DummyMountableBlockDevice(std::string b_block_path)
      : MountableBlockDevice(b_block_path) { }

  std::unique_ptr<const SectorSet> GetInUseSectors() {
    return nullptr;
  }

  void Freeze() { }
  void Thaw() { }
};

class DummyReplyChannel : public ReplyChannel {
 public:
  DummyReplyChannel() : is_available_(true) { }
  virtual void SendReply(const Reply &reply) { }
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

    source_device = std::make_shared<DummyMountableBlockDevice>(
                        source_loop->path());

    destination_device = std::make_shared<DummyMountableBlockDevice>(
                            destination_loop->path());

    source_manager = std::make_shared<UnsyncedSectorManager>(
                          source_loop->path());

    reply_channel = std::make_shared<DummyReplyChannel>();
  }

  void ConstructSynchronizer() {
    source_manager->StartTracer();
    device_synchronizer = std::make_shared<DeviceSynchronizer>(
                              source_device,
                              source_manager,
                              destination_device,
                              reply_channel);

  }

  ~DeviceSynchronizerTest() { }

  // Order matters here, things will be destructed in opposite order
  // of declaration

  std::shared_ptr<LoopDevice> source_loop;
  std::shared_ptr<MountableBlockDevice> source_device;
  std::shared_ptr<UnsyncedSectorManager> source_manager;

  std::shared_ptr<LoopDevice> destination_loop;
  std::shared_ptr<MountableBlockDevice> destination_device;

  std::shared_ptr<DummyReplyChannel> reply_channel;

  std::shared_ptr<DeviceSynchronizer> device_synchronizer;
};

}

TEST_F(DeviceSynchronizerTest, CanConstruct) {
  // Make sure there is something to sync
  source_manager->store()->AddUnsyncedInterval(SectorInterval(0, 1));
  ConstructSynchronizer();
  EXPECT_NE(nullptr, device_synchronizer);
}

TEST_F(DeviceSynchronizerTest, ConstructFailure1) {
  try {
    ConstructSynchronizer();
    FAIL() << "Construction succeeded but there was no data to sync";
  } catch (const std::runtime_error &e) {
    // good
  }
}

TEST_F(DeviceSynchronizerTest, ConstructFailure2) {
  source_manager->store()->AddUnsyncedInterval(SectorInterval(0, 1));
  source_device = destination_device;

  try {
    ConstructSynchronizer();
    FAIL() << "Construction succeeded but the devices were the same";
  } catch (const std::runtime_error &e) {
    // good
  }
}

TEST_F(DeviceSynchronizerTest, BasicSyncTest) {
  // Write garbage to the first 4k block then sync (which should overwrite it)
  // from source_device
  int destination_fd = destination_device->Open();
  int urandom_fd = open("/dev/urandom", O_RDONLY);
  char buf[4096];

  if (read(urandom_fd, buf, 4096) == -1) {
    FAIL() << "Failed reading from urandom";
  }
  if (write(destination_fd, buf, 4096) == -1) {
    FAIL() << "Failed writing to destination";
  }

  destination_device->Close();

  source_manager->store()->AddUnsyncedInterval(SectorInterval(0, 8));

  ConstructSynchronizer();

  device_synchronizer->StartSync();

  int wait_secs = 0;
  while (!device_synchronizer->done()) {
    sleep(1);
    wait_secs++;
    if (wait_secs > 10) {
      FAIL() << "Took way too long to sync a 4k block";
    }
  }

  EXPECT_TRUE(device_synchronizer->succeeded());
}

// TODO Make this test block size agnostic
TEST_F(DeviceSynchronizerTest, SyncTest) {
  // Write garbage to the first 4k block then sync (which should overwrite it)
  // from source_device
  int source_fd = source_device->Open();

  int urandom_fd = open("/dev/urandom", O_RDONLY);
  std::array<char, 4096> buf;

  // TODO Remove this once the block size is no longer hard coded
  ASSERT_EQ(4096UL, source_device->BlockSizeBytes());

  for (int i = 0; i < 5; i += 1) {
    if (read(urandom_fd, buf.data(), 4096) == -1) {
      FAIL() << "Failed reading from urandom";
    }

    if (write(source_fd, buf.data(), 4096) == -1) {
      FAIL() << "Failed writing to source";
    }
    // Only mark every other block to sync
    if (i % 2 == 0) {
      source_manager->store()->AddUnsyncedInterval(SectorInterval(i * 8, (i + 1) * 8));
    }
  }

  std::array<char, 4096> zero_array;
  // Make sure we wrote successfully
  lseek(source_fd, 0, SEEK_SET);
  for (int i = 0; i < 5; i += 1) {
    if (read(source_fd, buf.data(), 4096) == -1) {
      FAIL() << "Failed reading source";
    }
    ASSERT_NE(buf, zero_array);
  }

  close(urandom_fd);
  source_device->Close();

  ConstructSynchronizer();
  device_synchronizer->StartSync();

  int wait_secs = 0;
  while (!device_synchronizer->done()) {
    sleep(1);
    wait_secs++;
    if (wait_secs > 3) {
      FAIL() << "Took way too long to sync the blocks";
    }
  }

  source_fd = source_device->Open();
  int destination_fd = destination_device->Open();
  std::array<char, 4096> source_buf;
  std::array<char, 4096> destination_buf;

  // 0, 2, 4 should be synced, while 1, 3 should not
  for (int i = 0; i < 5; i += 1) {
    if (read(source_fd, source_buf.data(), 4096) == -1) {
      PLOG(ERROR) << "Read";
      FAIL() << "Failed reading source";
    }
    if (read(destination_fd, destination_buf.data(), 4096) == -1) {
      PLOG(ERROR) << "Read";
      FAIL() << "Failed reading destination";
    }

    if (i % 2 == 0) {
      EXPECT_EQ(source_buf, destination_buf) << "i is " << i;
    } else {
      EXPECT_NE(source_buf, destination_buf) << "i is " << i;
    }
  }
}
