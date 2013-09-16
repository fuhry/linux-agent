#include "block_trace/device_tracer_manager.h"

namespace datto_linux_client {

DeviceTracerManager::DeviceTracerManager() : tracers_(), tracers_mutex_() { }

void DeviceTracerManager::StartDeviceTracer(const std::string &block_dev_path) {
  std::lock_guard<std::mutex> lock_guard(tracers_mutex_);

  if (tracers_.count(block_dev_path) == 1) {
    // TODO: Better exception
    throw "block_dev_path is already being traced";
  }

  std::shared_ptr<DeviceTracer> trace_p(new DeviceTracer(block_dev_path));

  tracers_[block_dev_path] = trace_p;
}

void DeviceTracerManager::StopDeviceTracer(const std::string &block_dev_path) {
  std::lock_guard<std::mutex> lock_guard(tracers_mutex_);

  if (tracers_.count(block_dev_path) == 0) {
    // TODO: Better exception
    throw "block_dev_path is not being traced";
  }

  tracers_.erase(block_dev_path);
}

}
