#include "unsynced_sector_manager/unsynced_sector_manager.h"

#include <glog/logging.h>

#include "unsynced_sector_manager/unsynced_tracking_exception.h"
#include "block_trace/trace_handler.h"
#include "block_trace/device_tracer.h"

namespace datto_linux_client {

UnsyncedSectorManager::UnsyncedSectorManager(const std::string &block_dev_path)
    : block_dev_path_(block_dev_path),
      store_(),
      device_tracer_(nullptr) {
  store_ = std::make_shared<UnsyncedSectorStore>();
}

UnsyncedSectorManager::~UnsyncedSectorManager() {
  // The data structure destructors will cause the element destructors to run,
  // which will clean up everything
}

void UnsyncedSectorManager::FlushTracer() {
  device_tracer_->FlushBuffers();
}

void UnsyncedSectorManager::StartTracer() {
  if (device_tracer_) {
    LOG(ERROR) << "Already tracing " << block_dev_path_;
    throw UnsyncedTrackingException("Already tracing block device");
  }

  LOG(INFO) << "Starting tracing on " << block_dev_path_;
  std::shared_ptr<TraceHandler> trace_handler(new TraceHandler(store_));

  std::unique_ptr<DeviceTracer> device_tracer(
      new DeviceTracer(block_dev_path_, trace_handler));

  device_tracer_ = std::move(device_tracer);

  LOG(INFO) << "Started tracing on " << block_dev_path_;
}

void UnsyncedSectorManager::StopTracer() {
  // Tracer destructor will stop the tracer from running
  device_tracer_.reset();
}

} // datto_linux_client
