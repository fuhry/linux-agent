#ifndef DATTO_CLIENT_BLOCK_TRACE_BLOCK_TRACE_EXCEPTION_H_
#define DATTO_CLIENT_BLOCK_TRACE_BLOCK_TRACE_EXCEPTION_H_

#include <stdexcept>

namespace datto_linux_client {
class BlockTraceException : public std::runtime_error {
 public: 
  explicit BlockTraceException(const std::string &a_what)
    : runtime_error(a_what) {};
};
}

#endif //  DATTO_CLIENT_BLOCK_TRACE_BLOCK_TRACE_EXCEPTION_H_
