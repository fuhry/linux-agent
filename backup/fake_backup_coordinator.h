#ifndef DATTO_CLIENT_BACKUP_FAKE_BACKUP_COORDINATOR_H_
#define DATTO_CLIENT_BACKUP_FAKE_BACKUP_COORDINATOR_H_

#include <exception>

#include "backup/backup_coordinator.h"

namespace datto_linux_client {
// FakeBackupCoordinator exists to provide the minimal amount of
// functionality required for the fsimgcopy tool to use a
// DeviceSynchronizer
class FakeBackupCoordinator : public BackupCoordinator {
 public:
  FakeBackupCoordinator() {}
  ~FakeBackupCoordinator() {}

  virtual void SignalFinished() {}
  virtual bool SignalMoreWorkToDo() { return false; }
  virtual void AddFatalError(const BackupError &backup_error) {}
  virtual std::vector<BackupError> GetFatalErrors() const {
    throw std::runtime_error("Not implemented");
  }
  virtual void Cancel() {}
  virtual bool IsCancelled() const { return false; }
  virtual bool WaitUntilFinished(int timeout_millis) { return true; }

};
}
#endif //  DATTO_CLIENT_BACKUP_FAKE_BACKUP_COORDINATOR_H_
