#include <string>

#include <stdint.h>
#include <unistd.h>

namespace datto_linux_client {
class NbdServer {
 public:
  // Constructing will serve the file on a random port
  explicit NbdServer(const std::string &file_to_serve);
  // Constructing will serve the file on the port
  NbdServer(const std::string &file_to_serve, const uint16_t port);

  uint16_t port() {
    return port_;
  }
  // Destructor kills the nbd-server serving the file
  ~NbdServer();
 private:
  void Init(const std::string &file_to_serve, const uint16_t port);
  pid_t nbd_server_pid_;
  uint16_t port_;
};
} // datto_linux_client
