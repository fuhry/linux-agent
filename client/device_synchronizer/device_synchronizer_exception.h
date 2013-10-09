#ifndef DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_H_
#define DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_H_

#include <stdexcept>

namespace datto_linux_client {
class DeviceSynchronizerException : public std::runtime_error {
 public: 
  explicit DeviceSynchronizerException(const std::string &a_what)
    : runtime_error(a_what) { };
};
}

#endif //  DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_H_
