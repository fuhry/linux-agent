#include "block_device/device_mapper/dm_remove_task.h"

namespace datto_linux_client {

DmRemoveTask::DmRemoveTask(std::string device_name)
    : DmTask(DM_DEVICE_REMOVE) {

  DmTask::SetName(device_name);
}

void DmRemoveTask::Run() {
  DmTask::DoRun(true);
}

DmRemoveTask::~DmRemoveTask() { }

}

