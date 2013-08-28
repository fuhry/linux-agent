#include "block_device/device_mapper/dm_suspend_task.h"

namespace datto_linux_client {

DmSuspendTask::DmSuspendTask(std::string device_name)
    : DmTask(DM_DEVICE_SUSPEND) {

  DmTask::SetName(device_name);
}

void DmSuspendTask::Run() {
  DmTask::DoRun(false);
}

DmSuspendTask::~DmSuspendTask() { }

}
