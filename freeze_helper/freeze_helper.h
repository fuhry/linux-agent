#ifndef DATTO_CLIENT_FREEZE_HELPER_FREEZE_HELPER_H_
#define DATTO_CLIENT_FREEZE_HELPER_FREEZE_HELPER_H_

#include <atomic>
#include <thread>
#include <condition_variable>
#include <functional>
#include <mutex>

#include "block_device/mountable_block_device.h"

namespace datto_linux_client {

// FreezeHelper is designed to assist with situations where data needs to be
// read off of disk while the system is modifying that data.
//
// FreezeHelper will freeze the disk 
// 
// This class is NOT THREAD SAFE
class FreezeHelper {
 public:
  FreezeHelper(MountableBlockDevice &block_device, int freeze_time_millis);

  // This will unfreeze the MountableBlockDevice if it is currently frozen
  ~FreezeHelper();

  // Call this immediately before reading any data off disk that might
  // be volatile
  void BeginRequiredFreezeBlock();

  // Call this immediately after the read. This will return a boolean
  // indicating if the disk was frozen since the last call to
  // BeginRequiredFreezeBlock. This way, FreezeHelper can unfreeze the disk
  // if it has been frozen for too long and the caller can refreeze and
  // reread the data.
  bool EndRequiredFreezeBlock();

  void RunWhileFrozen(std::function<void()> to_run);

  FreezeHelper(const FreezeHelper &) = delete;
  FreezeHelper& operator=(const FreezeHelper &) = delete;
 private:
  MountableBlockDevice &block_device_;
  int freeze_time_millis_;
  std::thread unfreeze_thread_;
  std::atomic<bool> is_frozen_;
  std::atomic<bool> did_freeze_;
  std::atomic<bool> was_error_;

  std::exception_ptr freeze_exception_;

  std::condition_variable continue_var_;
  std::mutex continue_mutex_;
};
} // datto_linux_client

#endif //  DATTO_CLIENT_FREEZE_HELPER_FREEZE_HELPER_H_
