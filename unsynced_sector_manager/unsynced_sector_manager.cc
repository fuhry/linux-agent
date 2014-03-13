#include "unsynced_sector_manager/unsynced_sector_manager.h"

#include <glog/logging.h>
#include <sys/sysmacros.h>

#include "unsynced_sector_manager/unsynced_tracking_exception.h"
#include "block_trace/trace_handler.h"
#include "block_trace/device_tracer.h"

namespace {
  // number of a seconds ago a write should have happened to be
  // considered non-volatile
  const int VOLATILE_SECONDS = 10;
}

namespace datto_linux_client {

UnsyncedSectorManager::UnsyncedSectorManager() : store_map_(), tracer_map_() {}

UnsyncedSectorManager::~UnsyncedSectorManager() {
  // The data structure destructors will cause the element destructors to run,
  // which will clean up everything
}

void UnsyncedSectorManager::StartTracer(const BlockDevice &device) {
  if (tracer_map_[device.dev_t()]) {
    LOG(ERROR) << "Already tracing " << device.path();
    throw UnsyncedTrackingException("Already tracing block device");
  }

  LOG(INFO) << "Starting tracing on " << device.path();

  auto store = GetStore(device);
  auto device_tracer = CreateDeviceTracer(device.path(), store);

  store_map_[device.dev_t()] = store;
  tracer_map_[device.dev_t()] = std::move(device_tracer);
}

void UnsyncedSectorManager::StopTracer(const BlockDevice &device) {
  // Tracer destructor will stop the tracer from running
  tracer_map_[device.dev_t()] = nullptr;
}

void UnsyncedSectorManager::FlushTracer(const BlockDevice &device) {
  if (tracer_map_.count(device.dev_t())) {
    tracer_map_[device.dev_t()]->FlushBuffers();
  }
}

bool UnsyncedSectorManager::IsTracing(const BlockDevice &device) const {
  if (tracer_map_.count(device.dev_t())) {
    return tracer_map_.at(device.dev_t()) != nullptr;
  } else {
    return false;
  }
}

std::shared_ptr<UnsyncedSectorStore> UnsyncedSectorManager::GetStore(
    const BlockDevice &device) {
  if (!store_map_[device.dev_t()]) {
    store_map_[device.dev_t()] =
        std::make_shared<UnsyncedSectorStore>(VOLATILE_SECONDS);
  }
  return store_map_.at(device.dev_t());
}

std::shared_ptr<DeviceTracer> UnsyncedSectorManager::CreateDeviceTracer(
    const std::string &path,
    std::shared_ptr<UnsyncedSectorStore> store) {
  auto trace_handler = std::make_shared<TraceHandler>(store);
  std::shared_ptr<DeviceTracer> device_tracer(
      new DeviceTracer(path, trace_handler));
  return device_tracer;
}

} // datto_linux_client
