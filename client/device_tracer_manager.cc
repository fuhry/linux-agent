#include "block_trace/device_tracer_manager.h"

namespace datto_linux_client {

DeviceTracerManager::DeviceTracerManager(std::shared_ptr<TraceHandler> trace_handler)
    : tracers_(),
      tracers_mutex_(),
      trace_handler_(trace_handler) { }

void DeviceTracerManager::StartDeviceTracer(const std::string &block_dev_path) {
  std::lock_guard<std::mutex> lock_guard(tracers_mutex_);

  if (tracers_.count(block_dev_path) == 1) {
    // TODO: Better exception
    throw "block_dev_path is already being traced";
  }

  std::unique_ptr<DeviceTracer> device_tracer(new DeviceTracer(block_dev_path, trace_handler_));

  tracers_[block_dev_path] = std::move(device_tracer);
}

void DeviceTracerManager::StopDeviceTracer(const std::string &block_dev_path) {
  std::lock_guard<std::mutex> lock_guard(tracers_mutex_);

  if (tracers_.count(block_dev_path) == 0) {
    // TODO: Better exception
    throw "block_dev_path is not being traced";
  }

  // Removing the entry in the map will call the destructor of the DeviceTrace
  tracers_.erase(block_dev_path);
}

}
