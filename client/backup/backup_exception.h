#ifndef DATTO_CLIENT_BACKUP_BACKUP_EXCEPTION_H_
#define DATTO_CLIENT_BACKUP_BACKUP_EXCEPTION_H_

#include <stdexcept>

namespace datto_linux_client {
class BackupException : public std::runtime_error {
 public: 
  explicit BackupException(const std::string &a_what)
    : runtime_error(a_what) { };
};
}

#endif //  DATTO_CLIENT_BACKUP_BACKUP_EXCEPTION_H_
