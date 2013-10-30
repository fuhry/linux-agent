#ifndef DATTO_CLIENT_UNSYNCED_SECTOR_STORE_UNSYNCED_SECTOR_MANAGER_H_
#define DATTO_CLIENT_UNSYNCED_SECTOR_STORE_UNSYNCED_SECTOR_MANAGER_H_

#include <memory>
#include <string>
#include "unsynced_sector_manager/unsynced_sector_store.h"

namespace datto_linux_client {

class UnsyncedSectorManager {
 public:
  UnsyncedSectorManager();
  ~UnsyncedSectorManager();

  void StartTracer(const std::string &block_dev_path);
  void StopTracer(const std::string &block_dev_path);

  std::shared_ptr<UnsyncedSectorStore> GetStore(
      const std::string &block_dev_path) const;

  void StopAllTracers();

  UnsyncSectorManager (const UnsyncedSectorManager&) = delete;
  UnsyncSectorManager& operator=(const UnsyncedSectorManager&) = delete;
 private:
  std::map<const std::string, std::shared_ptr<UnsyncedSectorStore>>
    device_unsynced_stores_;
  std::map<const std::string, std::unique_ptr<DeviceTracer>>
    device_tracers_;
  mutable std::mutex maps_mutex_;
};

}

#endif //  DATTO_CLIENT_UNSYNCED_SECTOR_STORE_UNSYNCED_SECTOR_MANAGER_H_
