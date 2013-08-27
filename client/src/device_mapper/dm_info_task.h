#ifndef DATTO_CLIENT_DEVICE_MAPPER_DM_INFO_TASK_H_
#define DATTO_CLIENT_DEVICE_MAPPER_DM_INFO_TASK_H_

#include "device_mapper/dm_task.h"
#include "device_mapper/dm_info.h"

namespace datto_linux_client {

class DmInfoTask : public DmTask {
 public:
  DmInfoTask(std::string device_name);
  void Run();
  DmInfo info();
  ~DmInfoTask();
 private:
  bool was_run_;
};

}

#endif //  DATTO_CLIENT_DEVICE_MAPPER_DM_INFO_TASK_H_
