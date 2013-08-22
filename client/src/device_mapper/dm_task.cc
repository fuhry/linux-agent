#include "device_mapper/dm_task.h"

namespace datto_linux_client {

DmTask::DmTask(int task_type, bool use_udev) : use_udev_(use_udev) {
  dm_task_ = dm_task_create(task_type);
  if (!dm_task_) {
    // TODO Throw something useful
    throw "create";
  }
}

DmTask::~DmTask() {
  dm_task_destroy(dm_task_);
}

void DmTask::Run() {
  uint32_t udev_cookie = 0;

  if (use_udev_) {
    if (!dm_task_set_cookie(dm_task_, &udev_cookie, 0)) {
      // TODO Throw something useful
      throw "cookie";
    }
  }

  if (!dm_task_run(dm_task_)) {
    // TODO Throw something useful
    throw "run";
  }

  if (use_udev_) {
    dm_udev_wait(udev_cookie);
  }
}

}

