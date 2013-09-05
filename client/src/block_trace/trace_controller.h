#ifndef DATTO_CLIENT_BLOCK_TRACE_TRACE_CONTROLLER_H_
#define DATTO_CLIENT_BLOCK_TRACE_TRACE_CONTROLLER_H_

#include <boost/icl/interval.hpp>

namespace datto_linux_client {
class TraceController : private boost::noncopyable {
 public:
   TraceController();

   void StartDeviceTrace(const BlockDevice& block_device);
   void StopDeviceTrace(const BlockDevice& block_device);

   void CreateEmptyTrace(const BlockDevice& block_device);
   boost::icl::interval TakeTrace(const BlockDevice& block_device);

  ~TraceController();
 private:
}

#endif //  DATTO_CLIENT_BLOCK_TRACE_TRACE_CONTROLLER_H_
