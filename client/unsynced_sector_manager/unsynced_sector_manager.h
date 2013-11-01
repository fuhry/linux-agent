#ifndef DATTO_CLIENT_UNSYNCED_SECTOR_STORE_UNSYNCED_SECTOR_MANAGER_H_
#define DATTO_CLIENT_UNSYNCED_SECTOR_STORE_UNSYNCED_SECTOR_MANAGER_H_

#include <map>
#include <memory>
#include <string>

#include "block_trace/device_tracer.h"
#include "unsynced_sector_manager/unsynced_sector_store.h"

namespace datto_linux_client {

class UnsyncedSectorManager {
 public:
  UnsyncedSectorManager();
  ~UnsyncedSectorManager();

  void StartTracer(const std::string &block_dev_path);
  void StopTracer(const std::string &block_dev_path);

  std::shared_ptr<UnsyncedSectorStore> GetStore(
      const std::string &block_dev_path);

  void StopAllTracers();

  UnsyncedSectorManager (const UnsyncedSectorManager&) = delete;
  UnsyncedSectorManager& operator=(const UnsyncedSectorManager&) = delete;

 private:
  std::map<const std::string, std::shared_ptr<UnsyncedSectorStore>>
      device_unsynced_stores_;
  std::map<const std::string, std::unique_ptr<DeviceTracer>>
      device_tracers_;
  // Use this mutex for all structures for now
  mutable std::mutex maps_mutex_;
};

}

#endif //  DATTO_CLIENT_UNSYNCED_SECTOR_STORE_UNSYNCED_SECTOR_MANAGER_H_
