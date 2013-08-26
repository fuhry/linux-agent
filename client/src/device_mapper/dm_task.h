#ifndef DATTO_CLIENT_DEVICE_MAPPER_DM_TASK_H_
#define DATTO_CLIENT_DEVICE_MAPPER_DM_TASK_H_

extern "C" {
// Need to avoid C++ keyword 'private' collision in libdevmapper.h
#define private avoid_cxx_private_keyword
#include <libdevmapper.h>
#undef private
}

#include <string>
#include <vector>
#include <device_mapper/dm_target.h>

namespace datto_linux_client {

// The DmTask class provides two things.
// 1. An interface requiring a Run() method
// 2. Several helper methods for subclasses.
class DmTask {

 public:
  virtual void Run() = 0;
  virtual ~DmTask();

 protected:
  explicit DmTask(int task_type);

  struct dm_task *dm_task_;

  void DoRun(bool use_udev);
  void SetName(std::string device_name);
  void AddTargets(std::vector<DmTarget> targets);

 private:
  // A udev cookie is a semaphore that coordinates udev rules.
  // Only resume and remove should need to use this
  class UDevCookie {
   public:
    explicit UDevCookie(struct dm_task *task);
    ~UDevCookie();
   private:
    uint32_t cookie_;
  };

  int task_type_;
  bool use_udev_;

};

}

#endif  // DATTO_CLIENT_DEVICE_MAPPER_DM_TASK_H_
