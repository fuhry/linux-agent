#include <string>

#include <stdint.h>
#include <unistd.h>

namespace datto_linux_client {
class NbdServer {
 public:
  // Constructing will serve the file on the port
  NbdServer(const std::string &file_to_serve, const uint16_t port);
  // Destructor kills the nbd-server serving the file
  ~NbdServer();
 private:
  pid_t nbd_server_pid_;
};
} // datto_linux_client
