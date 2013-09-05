#ifndef DATTO_CLIENT_BLOCK_TRACE_TRACE_CONTROLLER_H_
#define DATTO_CLIENT_BLOCK_TRACE_TRACE_CONTROLLER_H_

#include <boost/icl/interval.hpp>
#include <boost/noncopyable.hpp>
#include <map>
#include <memory>
#include <mutex>

#include "device_trace.h"

namespace datto_linux_client {

class TraceController : private boost::noncopyable {
 public:
  TraceController();
 
  void StartDeviceTrace(const std::string &block_path);
  void StopDeviceTrace(const std::string &block_path);
 
  void CreateEmptyTrace(const std::string &block_path);
  boost::icl::interval<int> TakeTrace(const std::string &block_path);
 
  ~TraceController();

 private:
  std::map<const std::string, DeviceTrace> traces_;
};

}

#endif //  DATTO_CLIENT_BLOCK_TRACE_TRACE_CONTROLLER_H_
