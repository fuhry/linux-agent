#ifndef DATTO_CLIENT_REMOTE_BLOCK_DEVICE_NBD_CLIENT_H_
#define DATTO_CLIENT_REMOTE_BLOCK_DEVICE_NBD_CLIENT_H_

#include <stdint.h>
#include <atomic>
#include <string>
#include <thread>

namespace datto_linux_client {

class NbdClient {
 public:
  NbdClient(std::string host, uint16_t port);

  std::string host() const {
    return host_;
  }
  uint16_t port() const {
    return port_;
  }
  std::string nbd_device_path() const {
    return nbd_device_path_;
  }

  bool IsConnected();
  void Disconnect();

  ~NbdClient();

  NbdClient(const NbdClient &);
  NbdClient& operator=(const NbdClient &);
 private:
  void ConfigureNbdDevice();

  std::string host_;
  uint16_t port_;

  std::atomic<bool> disconnect_;

  void NbdDoIt();
  std::thread nbd_do_it_thread_;

  uint64_t block_device_size_;

  int nbd_fd_;
  int sock_;

  std::string nbd_device_path_;
};
}
#endif //  DATTO_CLIENT_REMOTE_BLOCK_DEVICE_NBD_CLIENT_H_
