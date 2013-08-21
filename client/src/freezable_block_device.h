#ifndef DATTO_CLIENT_FREEZABLE_BLOCK_DEVICE_H_
#define DATTO_CLIENT_FREEZABLE_BLOCK_DEVICE_H_

#include <string>
#include "block_device.h"

namespace datto_linux_client {
class FreezableBlockDevice : public BlockDevice {
  public:
    explicit FreezableBlockDevice(std::string block_path);
    ~FreezableBlockDevice();
    void Freeze();
    void Unfreeze();
    bool IsConsistent() const;
  private:
    bool is_frozen_;
};
}

#endif // DATTO_CLIENT_FREEZABLE_BLOCK_DEVICE_H_
