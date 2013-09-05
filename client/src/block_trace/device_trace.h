#ifndef DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACE_H_
#define DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACE_H_

#include <boost/icl/interval.hpp>
#include <boost/noncopyable.hpp>
#include <mutex>
#include <memory>

namespace datto_linux_client {

class DeviceTrace : private boost::noncopyable {
 public:
  DeviceTrace(const std::string &block_path);

  void AddTraceData(struct blk_io_trace &trace);

  void PushEmptyInterval();
  std::shared_ptr<const boost::icl::interval<int>> PopInterval();

  std::string block_path();

  ~DeviceTrace();

 private:
  std::string block_path_;

};

}

#endif //  DATTO_CLIENT_BLOCK_TRACE_DEVICE_TRACE_H_
