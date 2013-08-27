#include "device_mapper/dm_table_task.h"

#include <glog/logging.h>

namespace datto_linux_client {

DmTableTask::DmTableTask(std::string device_name)
    : DmTask(DM_DEVICE_TABLE),
      device_name_(device_name),
      targets_(),
      was_run_(false) {

  DmTask::SetName(device_name_);

}

void DmTableTask::Run() {
  VLOG(1) << "running device-mapper table task";

  DmTask::DoRun(false);

  // Populate targets_ with the results of the task
  void *next = NULL;
  uint64_t start = -1;
  uint64_t length = -1;
  char *target_type = NULL;
  char *params = NULL;

  do {
    next = dm_get_next_target(dm_task_, next, &start, &length,
        &target_type, &params);

    LOG(INFO) << device_name_ << " table: " << start << " " << length << " "
        << target_type << " " << params;

    DmTarget target = {start, length, target_type, params};
    targets_.push_back(target);
  } while(next);

  was_run_ = true;
}

std::vector<DmTarget> DmTableTask::targets() const {
  if (!was_run_) {
    // TODO: Throw something useful
    throw "not run";
  }

  return targets_;
}

DmTableTask::~DmTableTask() { }

}
