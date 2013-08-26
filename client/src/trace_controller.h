#ifndef DATTO_CLIENT_TRACE_CONTROLLER_H_
#define DATTO_CLIENT_TRACE_CONTROLLER_H_

#include <boost/icl/interval.hpp>

namespace datto_linux_client {
class TraceController {
  public:
    TraceController();
    ~TraceController();
    void AddTrace(std::string block_path);
    void PrepareForGet(std::string block_path);
    boost::icl::interval GetTrace(std::string block_path);
    void StopTrace(std::string block_path);
}
}

#endif // DATTO_CLIENT_TRACE_CONTROLLER_H_
