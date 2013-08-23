#ifndef DATTO_CLIENT_DEVICE_MAPPER_DM_TASK_H_
#define DATTO_CLIENT_DEVICE_MAPPER_DM_TASK_H_

extern "C" {
// Need to avoid C++ keyword 'private' collision in libdevmapper.h
#define private avoid_cxx_private_keyword
#include <libdevmapper.h>
#undef private
}

namespace datto_linux_client {

class DmTask {
 public:
  // All Run() calls should call dm_run_task at some point
  virtual void Run() = 0;
  virtual ~DmTask();
 protected:
  DmTask(int task_type);
  struct dm_task *dm_task_;
};
}

#endif  // DATTO_CLIENT_DEVICE_MAPPER_DM_TASK_H_
