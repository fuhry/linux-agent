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
    void Run();
    virtual ~DmTask();
  protected:
    // Resume and Remove should be the only two that set use_udev
    DmTask(int task_type, bool use_udev = false);
    struct dm_task *dm_task_;
  private:
    const bool use_udev_;

};
}

#endif  // DATTO_CLIENT_DEVICE_MAPPER_DM_TASK_H_
