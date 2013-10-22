#ifndef DATTO_CLIENT_REQUEST_LISTENER_REQUEST_LISTENER_EXCEPTION_H_
#define DATTO_CLIENT_REQUEST_LISTENER_REQUEST_LISTENER_EXCEPTION_H_

#include <stdexcept>

namespace datto_linux_client {
class RequestListenerException : public std::runtime_error {
 public: 
  explicit RequestListenerException(const std::string &a_what)
    : runtime_error(a_what) { };
};
}

#endif //  DATTO_CLIENT_REQUEST_LISTENER_REQUEST_LISTENER_EXCEPTION_H_
