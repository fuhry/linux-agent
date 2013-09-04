#include "block_device/device_mapper/dm_create_task.h"
#include <glog/logging.h>

namespace datto_linux_client {

DmCreateTask::DmCreateTask(std::string device_name,
                           std::vector<DmTarget> targets)
    : DmTask(DM_DEVICE_CREATE) {

  DmTask::SetName(device_name);
  DmTask::AddTargets(targets);
}

void DmCreateTask::Run() {
  DmTask::DoRun(false);
}

DmCreateTask::~DmCreateTask() { }

}
