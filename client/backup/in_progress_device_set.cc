#include "backup/in_progress_device_set.h"

#include <glog/logging.h>

#include "backup/backup_exception.h"

namespace datto_linux_client {

InProgressDeviceSet::InProgressDeviceSet()
    : set_mutex_(), set_() {}

// We are really lazy with performance in this class because this is not going
// to be used under any load at all
void InProgressDeviceSet::AddDeviceOrThrow(const BlockDevice &device) {
  std::lock_guard<std::mutex> lock(set_mutex_);
  if (set_.count(device.dev_t())) {
    LOG(ERROR) << "Device in set: " << device.path();
    throw BackupException("Device already in set");
  }
  set_.insert(device.dev_t());
}

void InProgressDeviceSet::RemoveDevice(const BlockDevice &device) {
  std::lock_guard<std::mutex> lock(set_mutex_);
  if (set_.erase(device.dev_t()) == 0) {
    LOG(ERROR) << "Device not found: " << device.path();
    throw BackupException("Unable to remove device");
  }
}

int InProgressDeviceSet::Count() {
  std::lock_guard<std::mutex> lock(set_mutex_);
  return set_.size();
}
} // datto_linux_client
