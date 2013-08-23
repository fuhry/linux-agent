#include "device_mapper/dm_target.h"

namespace datto_linux_client {

DmTarget::DmTarget(uint64_t _start, uint64_t _length,
    const char *_target_type, const char *_params)
  : start(_start),
    length(_length),
    target_type(_target_type),
    params(_params) { } 

}
