#ifndef DATTO_CLIENT_DATTOD_DATTOD_EXCEPTION_H_
#define DATTO_CLIENT_DATTOD_DATTOD_EXCEPTION_H_

#include <stdexcept>

namespace datto_linux_client {
class DattodException : public std::runtime_error {
 public: 
  explicit DattodException(const std::string &a_what)
    : runtime_error(a_what) {};
};
}

#endif //  DATTO_CLIENT_DATTOD_DATTOD_EXCEPTION_H_
