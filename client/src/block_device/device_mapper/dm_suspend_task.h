#ifndef DATTO_CLIENT_BLOCK_DEVICE_DEVICE_MAPPER_DM_SUSPEND_TASK_H_
#define DATTO_CLIENT_BLOCK_DEVICE_DEVICE_MAPPER_DM_SUSPEND_TASK_H_

#include "block_device/device_mapper/dm_task.h"

namespace datto_linux_client {

class DmSuspendTask : public DmTask {
 public:
  explicit DmSuspendTask(std::string device_name);
  void Run();
  ~DmSuspendTask();
};

}

#endif //  DATTO_CLIENT_BLOCK_DEVICE_DEVICE_MAPPER_DM_SUSPEND_TASK_H_
