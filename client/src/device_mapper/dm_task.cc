#include "device_mapper/dm_task.h"

#include <glog/logging.h>
#include <boost/shared_ptr.hpp>

namespace datto_linux_client {

DmTask::DmTask(int task_type) : task_type_(task_type) {

  VLOG(2) << "creating device-mapper task (type: " << task_type_ << ")";

  dm_task_ = dm_task_create(task_type_);
  if (!dm_task_) {
    // TODO Throw something useful
    throw "create";
  }

}

DmTask::~DmTask() {
  VLOG(2) << "destroying device-mapper task (type: " << task_type_ << ")";
  dm_task_destroy(dm_task_);
}

void DmTask::DoRun(bool use_udev) {
  VLOG(1) << "running device-mapper task (type: " << task_type_ << ")";

  // shared_ptr cleans up when udev_cookie falls out of scope
  boost::shared_ptr<UDevCookie> udev_cookie;
  if (use_udev) {
    udev_cookie.reset(new UDevCookie(dm_task_));
  }

  if (!dm_task_run(dm_task_)) {
    // TODO: Throw something useful
    throw "run";
  }
}

void DmTask::SetName(std::string device_name) {

  if (!dm_task_set_name(dm_task_, device_name.c_str())) {
    // TODO: Throw something useful
    throw "set_name";
  }

}

void DmTask::AddTargets(std::vector<DmTarget> targets) {
  // Populate the task with targets
  std::vector<DmTarget>::iterator itr;
  for (itr = targets.begin(); itr != targets.end(); ++itr) {
    dm_task_add_target(dm_task_, itr->start, itr->length,
        itr->target_type, itr->params);
  }
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
