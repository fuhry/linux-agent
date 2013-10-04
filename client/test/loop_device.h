#include <string>
#include <sys/types.h>
#include <unistd.h>

namespace datto_linux_client_test {

// TODO: This class assumes that there is a /dev/shm and /tmp is the
// temporary directory. These assumptions should be made more explicit
// or removed.
static const char TEST_LOOP_SHARED_MEMORY[] = "/dev/shm/test_loop_path";

class LoopDevice {
 public:
  LoopDevice();

  void Sync();

  ~LoopDevice();

  std::string path() const {
    return path_;
  }

  size_t block_size() const {
    return block_size_;
  }

 private:
  std::string path_;
  size_t block_size_;
};

}
