#ifndef DATTO_CLIENT_BACKUP_H_
#define DATTO_CLIENT_BACKUP_H_

#include "block_device.h"

namespace datto_linux_client {

enum BackupStatus {
  NOT_STARTED = 0,
  PREPARING,
  COPYING,
  CLEANING_UP,
  FINISHED,
  FAILED,
}

class Backup {
 public:
  void DoBackup();
  BackupStatus status();
  virtual ~Backup();

  Backup(const Backup &) = delete;
  Backup& operator=(const Backup &) = delete;
 protected:
  virtual void Prepare() = 0;
  virtual void Copy() = 0;
  virtual void Cleanup() = 0;
 private:
  BackupStatus status_;
}

}

#endif  // DATTO_CLIENT_BACKUP_H_
