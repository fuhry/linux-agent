#ifndef DATTO_CLIENT_UNSYNCED_SECTOR_MANAGER_UNSYNCED_TRACKING_EXCEPTION_H_
#define DATTO_CLIENT_UNSYNCED_SECTOR_MANAGER_UNSYNCED_TRACKING_EXCEPTION_H_

#include <stdexcept>

namespace datto_linux_client {
class UnsyncedTrackingException : public std::runtime_error {
 public: 
  explicit UnsyncedTrackingException(const std::string &a_what)
    : runtime_error(a_what) {};
};
}

#endif //  DATTO_CLIENT_UNSYNCED_SECTOR_MANAGER_UNSYNCED_TRACKING_EXCEPTION_H_
