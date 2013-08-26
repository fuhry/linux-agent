#ifndef DATTO_CLIENT_DEVICE_MAPPER_DM_REMOVE_TASK_H_
#define DATTO_CLIENT_DEVICE_MAPPER_DM_REMOVE_TASK_H_

#include "device_mapper/dm_task.h"

namespace datto_linux_client {

class DmRemoveTask : public DmTask {
 public:
  explicit DmRemoveTask(std::string device_name);
  void Run();
  ~DmRemoveTask();
};

}

#endif //  DATTO_CLIENT_DEVICE_MAPPER_DM_REMOVE_TASK_H_
