#include "remote_block_device/nbd_block_device.h"
#include "remote_block_device/remote_block_device_exception.h"

#include <glog/logging.h>
#include <unistd.h>

namespace datto_linux_client {

NbdBlockDevice::NbdBlockDevice(std::string remote_host, uint16_t remote_port)
    : RemoteBlockDevice() {
  DLOG(INFO) << "Creating nbd client for host " << remote_host
             << " and port " << remote_port;
  nbd_client_ = std::unique_ptr<NbdClient>(new NbdClient(remote_host,
                                                         remote_port));

  path_ = (nbd_client_)->nbd_device_path();
  LOG(INFO) << "NBD Path: " << path_;
  // Setup major_, minor_, etc
  Init();
}

bool NbdBlockDevice::IsConnected() const {
  return nbd_client_->IsConnected();
}

void NbdBlockDevice::Disconnect() {
  LOG(INFO) << "Disconnecting NbdBlockDevice " << path_;
  nbd_client_->Disconnect();
}

NbdBlockDevice::~NbdBlockDevice() {
  try {
    Disconnect();
  } catch (const std::runtime_error &e) {
    LOG(ERROR) << e.what();
  }
}

}
