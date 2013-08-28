#ifndef DATTO_CLIENT_DEVICE_MAPPER_DM_TABLE_TASK_H_
#define DATTO_CLIENT_DEVICE_MAPPER_DM_TABLE_TASK_H_

#include "block_device/device_mapper/dm_task.h"

namespace datto_linux_client {

class DmTableTask : public DmTask {
 public:
  explicit DmTableTask(std::string device_name);
  std::vector<DmTarget> targets() const;
  void Run();
  ~DmTableTask();
 private:
  const std::string device_name_;
  std::vector<DmTarget> targets_;
  bool was_run_;
};

}

#endif //  DATTO_CLIENT_DEVICE_MAPPER_DM_TABLE_TASK_H_
