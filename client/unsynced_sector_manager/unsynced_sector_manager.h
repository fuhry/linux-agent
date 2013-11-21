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
  UnsyncedSectorManager(const std::string &block_dev_path);
  ~UnsyncedSectorManager();

  void FlushTracer();

  void StartTracer();
  void StopTracer();
  bool IsTracing();

  void SyncComplete();
  void ResetSynced();

  std::shared_ptr<UnsyncedSectorStore> store() {
    return store_;
  }

  UnsyncedSectorManager (const UnsyncedSectorManager&) = delete;
  UnsyncedSectorManager& operator=(const UnsyncedSectorManager&) = delete;

 private:
  const std::string block_dev_path_;
  std::shared_ptr<UnsyncedSectorStore> store_;
  std::unique_ptr<DeviceTracer> device_tracer_;
};

}

#endif //  DATTO_CLIENT_UNSYNCED_SECTOR_STORE_UNSYNCED_SECTOR_MANAGER_H_
