#include "freeze_helper/freeze_helper.h"

namespace datto_linux_client {

FreezeHelper::FreezeHelper(MountableBlockDevice &block_device,
                           int freeze_time_millis)
    : block_device_(block_device),
      freeze_time_millis_(freeze_time_millis),
      unfreeze_thread_(),
      is_frozen_(false),
      was_error_(false) {}

FreezeHelper::~FreezeHelper() {
  if (unfreeze_thread_.joinable()) {
    unfreeze_thread_.join();
  }
}

void FreezeHelper::RunWhileFrozen(std::function<void()> to_run) {
  int freeze_attempts = 0;
  do {
    freeze_attempts++;
    if (freeze_attempts > 10) {
      throw BlockDeviceException("Unable to read off of disk quickly enough");
    }
    BeginRequiredFreezeBlock();
    to_run();
  } while (!EndRequiredFreezeBlock());
}

void FreezeHelper::BeginRequiredFreezeBlock() {
  if (is_frozen_) {
    return;
  }

  if (unfreeze_thread_.joinable()) {
    unfreeze_thread_.join();
  }

  std::unique_lock<std::mutex> lock(continue_mutex_);

  did_freeze_ = false;
  was_error_ = false;
  unfreeze_thread_ = std::thread([&]() {
    try {
      block_device_.Freeze();
      did_freeze_ = true;
      is_frozen_ = true;
      continue_var_.notify_one();

      usleep(freeze_time_millis_ * 1000);

      is_frozen_ = false;
      block_device_.Thaw();
    } catch (const std::exception &e) {
      // parent thread reraise exception
      freeze_exception_ = std::current_exception();
      was_error_ = true;
      continue_var_.notify_one();
    }
  });

  // can't use is_frozen_ due to low-chance race of thaw happening
  // before this thread is woken up
  while (!did_freeze_ && !was_error_) {
    continue_var_.wait(lock);
  }

  if (was_error_) {
    std::rethrow_exception(freeze_exception_);
  }
}

bool FreezeHelper::EndRequiredFreezeBlock() {
  return is_frozen_;
}

} // datto_linux_client
