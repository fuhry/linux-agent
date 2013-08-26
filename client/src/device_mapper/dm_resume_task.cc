#include "device_mapper/dm_resume_task.h"

namespace datto_linux_client {

DmResumeTask::DmResumeTask(std::string device_name)
    : DmTask(DM_DEVICE_SUSPEND) {

  DmTask::SetName(device_name);
}

void DmResumeTask::Run() {
  DmTask::DoRun(true);
}

DmResumeTask::~DmResumeTask() { }

}
