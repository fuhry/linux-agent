#include "device_mapper/dm_task.h"

#include <glog/logging.h>

namespace datto_linux_client {

DmTask::DmTask(int task_type) {
  VLOG(2) << "creating device-mapper task";
  dm_task_ = dm_task_create(task_type);
  if (!dm_task_) {
    // TODO Throw something useful
    throw "create";
  }
}

DmTask::~DmTask() {
  VLOG(2) << "destroying device-mapper task";
  dm_task_destroy(dm_task_);
}

DmTask::UDevCookie::UDevCookie(struct dm_task *task) {
  uint16_t flags = 0;
  // dm_task_set_cookie initializes cookie_ for us
  if (!dm_task_set_cookie(task, &cookie_, flags)) {
    throw "unable to set cookie";
  }

}

DmTask::UDevCookie::~UDevCookie() {
  dm_udev_wait(cookie_);
}

}
