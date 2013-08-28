#ifndef DATTO_CLIENT_BLOCK_DEVICE_DEVICE_MAPPER_DM_INFO_H_
#define DATTO_CLIENT_BLOCK_DEVICE_DEVICE_MAPPER_DM_INFO_H_
namespace datto_linux_client {

struct DmInfo {
 public:
  bool exists;
  bool suspended;
  bool live_table;
  bool inactive_table;
  int32_t open_count;
  uint32_t event_nr;
  uint32_t major;
  uint32_t minor;
  bool read_only;
  int32_t target_count;
};

}

#endif //  DATTO_CLIENT_BLOCK_DEVICE_DEVICE_MAPPER_DM_INFO_H_
