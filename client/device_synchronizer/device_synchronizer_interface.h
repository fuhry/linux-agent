#ifndef DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_INTERFACE_H_
#define DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_INTERFACE_H_

#include <memory>

#include "backup/backup_coordinator.h"
#include "backup_status_tracker/backup_event_handler.h"

namespace datto_linux_client {

// Existance of this class allows for easier mocking in unit tests
class DeviceSynchronizerInterface {
 public:
  // Precondition: source_device must be both traced and mounted
  virtual void DoSync(std::shared_ptr<BackupCoordinator> coordinator,
                      std::shared_ptr<BackupEventHandler> event_handler) = 0;

  virtual ~DeviceSynchronizerInterface() {}
};
}

#endif //  DATTO_CLIENT_DEVICE_SYNCHRONIZER_DEVICE_SYNCHRONIZER_INTERFACE_H_
