#ifndef DATTO_CLIENT_BACKUP_IN_PROGRESS_DEVICE_SET_H_
#define DATTO_CLIENT_BACKUP_IN_PROGRESS_DEVICE_SET_H_

#include "block_device/block_device.h"
#include <sys/types.h>

#include <mutex>
#include <string>
#include <set>

namespace datto_linux_client {

class InProgressDeviceSet {
 public:
  InProgressDeviceSet();

  void AddDeviceOrThrow(const BlockDevice &device);
  void RemoveDevice(const BlockDevice &device);

  int Count();

  InProgressDeviceSet(const InProgressDeviceSet &) = delete;
  InProgressDeviceSet& operator=(const InProgressDeviceSet &) = delete;

 private:
  std::mutex set_mutex_;
  std::set<dev_t> set_;
};

} // datto_linux_client

#endif  // DATTO_CLIENT_BACKUP_IN_PROGRESS_DEVICE_SET_H_
