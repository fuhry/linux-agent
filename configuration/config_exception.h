#ifndef DATTO_CLIENT_CONFIGURATION_CONFIG_EXCEPTION_H_
#define DATTO_CLIENT_CONFIGURATION_CONFIG_EXCEPTION_H_

#include <stdexcept>

namespace datto_linux_client {
class ConfigException : public std::runtime_error {
 public: 
  explicit ConfigException(const std::string &a_what)
    : runtime_error(a_what) {};
};
}

#endif //  DATTO_CLIENT_CONFIGURATION_CONFIG_EXCEPTION_H_
