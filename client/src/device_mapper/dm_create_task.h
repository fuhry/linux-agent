#ifndef DATTO_CLIENT_DEVICE_MAPPER_DM_CREATE_TASK_H_
#define DATTO_CLIENT_DEVICE_MAPPER_DM_CREATE_TASK_H_

#include "device_mapper/dm_task.h"

namespace datto_linux_client {

class DmCreateTask : public DmTask {
 public:
  DmCreateTask(std::string device_name, std::vector<DmTarget> targets);
  void Run();
  ~DmCreateTask();
};

}

#endif //  DATTO_CLIENT_DEVICE_MAPPER_DM_CREATE_TASK_H_
