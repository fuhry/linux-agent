#ifndef DATTO_CLIENT_UNSYNCED_SECTOR_MANAGER_UNSYNCED_SECTOR_MANAGER_H_
#define DATTO_CLIENT_UNSYNCED_SECTOR_MANAGER_UNSYNCED_SECTOR_MANAGER_H_

#include <linux/types.h>
#include <map>
#include <memory>
#include <string>

#include "block_device/block_device.h"
#include "tracing/device_tracer.h"
#include "unsynced_sector_manager/unsynced_sector_store.h"

namespace datto_linux_client {

// This class is not thread safe
class UnsyncedSectorManager {
 public:
  UnsyncedSectorManager();
  virtual ~UnsyncedSectorManager();

  virtual void StartTracer(const BlockDevice &device);
  virtual void StopTracer(const BlockDevice &device);
  virtual bool IsTracing(const BlockDevice &device) const;
  virtual void FlushTracer(const BlockDevice &device);

  virtual std::shared_ptr<UnsyncedSectorStore> GetStore(
      const BlockDevice &device);

  UnsyncedSectorManager(const UnsyncedSectorManager&) = delete;
  UnsyncedSectorManager& operator=(const UnsyncedSectorManager&) = delete;

 protected:
  // Virtual to allow overriding in tests
  virtual std::shared_ptr<DeviceTracer> CreateDeviceTracer(
      const std::string &path, std::shared_ptr<UnsyncedSectorStore> store);

 private:
  std::map<dev_t, std::shared_ptr<UnsyncedSectorStore>> store_map_;
  std::map<dev_t, std::shared_ptr<DeviceTracer>> tracer_map_;
};

}

#endif //  DATTO_CLIENT_UNSYNCED_SECTOR_MANAGER_UNSYNCED_SECTOR_MANAGER_H_
