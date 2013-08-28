#ifndef DATTO_CLIENT_DEVICE_MAPPER_DM_RELOAD_TASK_H_
#define DATTO_CLIENT_DEVICE_MAPPER_DM_RELOAD_TASK_H_

#include "block_device/device_mapper/dm_task.h"

namespace datto_linux_client {

class DmReloadTask : public DmTask {
 public:
  DmReloadTask(std::string device_name, std::vector<DmTarget> targets);
  void Run();
  ~DmReloadTask();
};

}

#endif //  DATTO_CLIENT_DEVICE_MAPPER_DM_RELOAD_TASK_H_
