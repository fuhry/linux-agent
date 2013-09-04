#include "block_device/device_mapper/dm_reload_task.h"
#include <glog/logging.h>

namespace datto_linux_client {

DmReloadTask::DmReloadTask(std::string device_name,
                           std::vector<DmTarget> targets)
    : DmTask(DM_DEVICE_RELOAD) {

  DmTask::SetName(device_name);
  DmTask::AddTargets(targets);

}

void DmReloadTask::Run() {
  DmTask::DoRun(true);
}

DmReloadTask::~DmReloadTask() { }

}
