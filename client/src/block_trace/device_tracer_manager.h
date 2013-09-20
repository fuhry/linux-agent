#ifndef DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACER_MANAGER_H_
#define DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACER_MANAGER_H_

#include <boost/noncopyable.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "block_trace/device_tracer.h"

namespace datto_linux_client {

class DeviceTracerManager : private boost::noncopyable {
 public:
  DeviceTracerManager(std::shared_ptr<TraceHandler> trace_handler);
 
  void StartDeviceTracer(const std::string &block_dev_path);
  void StopDeviceTracer(const std::string &block_dev_path);

  ~DeviceTracerManager();

 private:
  std::map<const std::string, std::unique_ptr<DeviceTracer>> tracers_;
  std::mutex tracers_mutex_;
  std::shared_ptr<TraceHandler> trace_handler_;
};

}

#endif //  DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACER_MANAGER_H_
