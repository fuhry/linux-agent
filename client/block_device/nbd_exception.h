#ifndef DATTO_CLIENT_REMOTE_BLOCK_DEVICE_NBD_EXCEPTION_H
#define DATTO_CLIENT_REMOTE_BLOCK_DEVICE_NBD_EXCEPTION_H

#include <stdexcept>

namespace datto_linux_client {
class NbdException : public std::runtime_error {
 public: 
  explicit NbdException(const std::string &a_what)
    : runtime_error(a_what) { };
};
}

#endif //  DATTO_CLIENT_REMOTE_BLOCK_DEVICE_NBD_EXCEPTION_H_
