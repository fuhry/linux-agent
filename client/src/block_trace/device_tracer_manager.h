#ifndef DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACER_MANAGER_H_
#define DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACER_MANAGER_H_

#include <boost/icl/interval.hpp>
#include <boost/noncopyable.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <stdint.h>

#include "block_trace/device_tracer.h"

namespace datto_linux_client {

class DeviceTracerManager : private boost::noncopyable {
 public:
  DeviceTracerManager();
 
  void StartDeviceTracer(const std::string &block_dev_path);
  void StopDeviceTracer(const std::string &block_dev_path);
 
  std::shared_ptr<DeviceTracer> GetTracer(const std::string &block_dev_path);

  ~DeviceTracerManager();

 private:
  std::map<const std::string, std::shared_ptr<DeviceTracer>> tracers_;
  std::mutex tracers_mutex_;
};

}

#endif //  DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACER_MANAGER_H_
