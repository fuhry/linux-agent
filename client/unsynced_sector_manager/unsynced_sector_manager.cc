#include "unsynced_sector_manager/unsynced_sector_manager.h"

#include <glog/logging.h>

#include "unsynced_sector_manager/unsynced_tracking_exception.h"
#include "block_trace/trace_handler.h"

namespace datto_linux_client {

UnsyncedSectorManager::UnsyncedSectorManager()
    : device_unsynced_stores_(),
      device_tracers_(),
      maps_mutex_() {}

UnsyncedSectorManager::~UnsyncedSectorManager() {
  // The data structure destructors will cause the element destructors to run,
  // which will clean up everything
}

void UnsyncedSectorManager::StartTracer(const std::string &block_dev_path) {
  std::lock_guard<std::mutex> lock(maps_mutex_);

  if (device_tracers_.count(block_dev_path)) {
    LOG(ERROR) << "Attempt to trace an already traced device: "
                << block_dev_path;
    throw UnsyncedTrackingException("Device is already being traced");
  }

  LOG(INFO) << "Starting tracing on " << block_dev_path;
  std::shared_ptr<UnsyncedSectorStore> store(new UnsyncedSectorStore());
  std::shared_ptr<TraceHandler> trace_handler(new TraceHandler(store));

  std::unique_ptr<DeviceTracer> device_tracer(
      new DeviceTracer(block_dev_path, trace_handler));

  device_unsynced_stores_[block_dev_path] = store;
  device_tracers_[block_dev_path] = std::move(device_tracer);
}

void UnsyncedSectorManager::StopTracer(const std::string &block_dev_path) {
  std::lock_guard<std::mutex> lock(maps_mutex_);

  LOG(INFO) << "Stopping tracing on " << block_dev_path;
  // Tracer destructor will stop the tracer from running
  device_tracers_.erase(block_dev_path);
}

std::shared_ptr<UnsyncedSectorStore> UnsyncedSectorManager::GetStore(
      const std::string &block_dev_path) {
  std::lock_guard<std::mutex> lock(maps_mutex_);
  
  // Create the store if it doesn't exist
  if (!device_unsynced_stores_.count(block_dev_path)) {
    std::shared_ptr<UnsyncedSectorStore> store(new UnsyncedSectorStore());
    device_unsynced_stores_[block_dev_path] = store;
  }

  return device_unsynced_stores_.at(block_dev_path);
}

void UnsyncedSectorManager::StopAllTracers() {
  std::lock_guard<std::mutex> lock(maps_mutex_);
  LOG(INFO) << "Stopping all tracers";
  // Tracer destructors will stop everything
  device_tracers_.clear();
}

} // datto_linux_client
