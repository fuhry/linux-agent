#include "device_mapper/dm_info_task.h"
#include <glog/logging.h>

namespace datto_linux_client {

DmInfoTask::DmInfoTask(std::string device_name)
    : DmTask(DM_DEVICE_INFO),
      was_run_(false) {

  DmTask::SetName(device_name);
}

void DmInfoTask::Run() {
  DmTask::DoRun(false);
  was_run_ = true;
}

DmInfo DmInfoTask::info() {
  if (!was_run_) {
    // TODO: Throw something useful
    throw "not run";
  }

  struct dm_info dmi_s;
  if (!dm_task_get_info(dm_task_, &dmi_s)) {
    // TODO: Throw something useful
    throw "get_info";
  }
  
  DmInfo dmi = {
    dmi_s.exists,
    dmi_s.suspended,
    dmi_s.live_table,
    dmi_s.inactive_table,

    dmi_s.open_count,
    dmi_s.event_nr,
    dmi_s.major,
    dmi_s.minor,

    dmi_s.read_only,
    dmi_s.target_count
  };

  return dmi;
}

DmInfoTask::~DmInfoTask() { }

}
