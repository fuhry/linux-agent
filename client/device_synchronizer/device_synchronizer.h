#ifndef DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_H_
#define DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_H_

#include <memory>
#include <thread>
#include <atomic>
#include <unistd.h>
#include <stdint.h>

#include "block_device/block_device.h"
#include "block_device/mountable_block_device.h"
#include "request_listener/reply_channel.h"
#include "unsynced_sector_tracker/unsynced_sector_tracker.h"

namespace datto_linux_client {

class DeviceSynchronizer {
 public:
  DeviceSynchronizer(std::shared_ptr<MountableBlockDevice> source_device,
                     std::shared_ptr<UnsyncedSectorTracker> sector_tracker,
                     std::shared_ptr<BlockDevice> destination_device,
                     std::shared_ptr<ReplyChannel> reply_channel);
  void StartSync();
  void Stop();

  bool done() {
    return done_;
  }

  bool succeeded() {
    return succeeded_;
  }

 private:
  std::atomic<bool> should_stop_;
  std::atomic<bool> succeeded_;
  std::atomic<bool> done_;
  std::thread sync_thread_;

  std::shared_ptr<MountableBlockDevice> source_device_;
  std::shared_ptr<UnsyncedSectorTracker> sector_tracker_;
  std::shared_ptr<BlockDevice> destination_device_;
  std::shared_ptr<ReplyChannel> reply_channel_;
};
}

#endif //  DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_H_
