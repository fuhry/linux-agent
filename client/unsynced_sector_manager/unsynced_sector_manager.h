#ifndef DATTO_CLIENT_UNSYNCED_SECTOR_STORE_UNSYNCED_SECTOR_MANAGER_H_
#define DATTO_CLIENT_UNSYNCED_SECTOR_STORE_UNSYNCED_SECTOR_MANAGER_H_

#include <linux/types.h>
#include <map>
#include <memory>
#include <string>

#include "block_device/block_device.h"
#include "block_trace/device_tracer.h"
#include "unsynced_sector_manager/unsynced_sector_store.h"

namespace datto_linux_client {

// This class is not thread safe
class UnsyncedSectorManager {
 public:
  UnsyncedSectorManager();
  virtual ~UnsyncedSectorManager();

  void StartTracer(const BlockDevice &device);
  void StopTracer(const BlockDevice &device);
  bool IsTracing(const BlockDevice &device);

  std::shared_ptr<UnsyncedSectorStore> GetStore(const BlockDevice &device);

  UnsyncedSectorManager(const UnsyncedSectorManager&) = delete;
  UnsyncedSectorManager& operator=(const UnsyncedSectorManager&) = delete;

 protected:
  // Virtual to allow overriding in tests
  virtual std::unique_ptr<DeviceTracer> CreateDeviceTracer(
      std::string path, std::shared_ptr<UnsyncedSectorStore> store);

 private:
  std::map<dev_t, std::shared_ptr<UnsyncedSectorStore>> store_map_;
  std::map<dev_t, std::shared_ptr<DeviceTracer>> tracer_map_;
};

}

#endif //  DATTO_CLIENT_UNSYNCED_SECTOR_STORE_UNSYNCED_SECTOR_MANAGER_H_
