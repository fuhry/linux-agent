#ifndef DATTO_CLIENT_TEST_LOOP_DEVICE_H_
#define DATTO_CLIENT_TEST_LOOP_DEVICE_H_

#include <string>
#include <sys/types.h>
#include <unistd.h>

namespace datto_linux_client_test {

// TODO: This class assumes that there is a /dev/shm this should be made
// more explicit or removed.
static const char TEST_LOOP_SHARED_MEMORY[] = "/dev/shm/test_loop_path";

class LoopDevice {
 public:
  LoopDevice();
  explicit LoopDevice(std::string backing_file_path);

  void FormatAsExt3();
  void FormatAsXfs();
  
  void Sync();

  ~LoopDevice();

  std::string path() const {
    return path_;
  }

  size_t block_size() const {
    return block_size_;
  }

 private:
  void Init();
  void SetRandNum();
  std::string GetSharedMemoryPath();

  std::string path_;
  size_t block_size_;
  bool is_backing_path_;
  int rand_num_;
};

}

#endif //  DATTO_CLIENT_TEST_LOOP_DEVICE_H_
