#ifndef DATTO_CLIENT_BACKUP_H_
#define DATTO_CLIENT_BACKUP_H_

#include "block_device/block_device.h"

namespace datto_linux_client {

class FullBackup : public Backup {
  FullBackup(std::string source_device, std::string destination_device);

  void Prepare();
  void Copy();
  void Cleanup();
};

} // datto_linux_client

#endif  // DATTO_CLIENT_BACKUP_H_
