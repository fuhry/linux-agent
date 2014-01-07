#include "backup/backup_coordinator.h"

#include <chrono>

#include <glog/logging.h>

namespace datto_linux_client {

BackupCoordinator::BackupCoordinator(int num_workers)
    : count_(num_workers),
      mutex_(),
      cond_variable_(),
      cancelled_(false),
      fatal_error_() {}

void BackupCoordinator::SignalFinished() {
  std::lock_guard<std::mutex> lock(mutex_);
  --count_;
  cond_variable_.notify_all();
}

bool BackupCoordinator::SignalMoreWorkToDo() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (cancelled_ || count_ == 0) {
    return false;
  }
  ++count_;
  return true;
}

void BackupCoordinator::SetFatalError(const std::exception &exception) {
  std::lock_guard<std::mutex> lock(mutex_);
  fatal_error_ = std::make_shared<std::exception>(exception);
  cancelled_ = true;
  cond_variable_.notify_all();
}

std::shared_ptr<std::exception> BackupCoordinator::GetFatalError() {
  std::lock_guard<std::mutex> lock(mutex_);
  return fatal_error_;
}

void BackupCoordinator::Cancel() {
  std::lock_guard<std::mutex> lock(mutex_);
  cancelled_ = true;
  cond_variable_.notify_all();
}

bool BackupCoordinator::IsCancelled() {
  std::lock_guard<std::mutex> lock(mutex_);
  return cancelled_;
}

bool BackupCoordinator::WaitUntilFinished(int timeout_millis) {
  CHECK_GT(timeout_millis, 0) << "timeout is not positive: " << timeout_millis;

  std::unique_lock<std::mutex> lock(mutex_);
  auto condition = [&]() { return count_ == 0 || cancelled_; };
  auto timeout = std::chrono::milliseconds(timeout_millis);
  return cond_variable_.wait_for(lock, timeout, condition);
}

}
