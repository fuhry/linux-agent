#ifndef DATTO_CLIENT_DEVICE_MAPPER_DM_TARGET_H_
#define DATTO_CLIENT_DEVICE_MAPPER_DM_TARGET_H_

#include <stdint.h>

namespace datto_linux_client {

struct DmTarget {
 public:
  uint64_t start;
  uint64_t length;
  const char *target_type;
  const char *params;
};

}

#endif //  DATTO_CLIENT_DEVICE_MAPPER_DM_TARGET_H_
