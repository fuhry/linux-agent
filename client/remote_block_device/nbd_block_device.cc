#include "remote_block_device/nbd_block_device.h"
#include "remote_block_device/remote_block_device_exception.h"

#include <fcntl.h>
#include <glog/logging.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace datto_linux_client {

// TODO: If this ends up making it to prod, we should stop using system
// and start using exec(). As I *think* this is temporary, don't worry about
// this yet.
NbdBlockDevice::NbdBlockDevice(std::string remote_host, uint16_t remote_port)
    : RemoteBlockDevice() {
  nbd_client_ = std::unique_ptr<NbdClient>(new NbdClient(remote_host,
                                                         remote_port));

  BlockDevice::block_path_ = (nbd_client_)->nbd_device_path();
  // Setup major_, minor_, etc
  BlockDevice::Init();
}

bool NbdBlockDevice::IsConnected() const {
  std::ostringstream nbd_command_stream;

  nbd_command_stream << "nbd-client -c " << block_path_;
  int ret_val = system(nbd_command_stream.str().c_str());

  switch (ret_val) {
    case 0:
      return true;
    case 1:
      return false;
    case 2:
      PLOG(ERROR) << nbd_command_stream.str();
      throw RemoteBlockDeviceException("Error while checking nbd connection");
    case 127:
      PLOG(ERROR) << "system returned 127";
      PLOG(ERROR) << nbd_command_stream.str();
      throw RemoteBlockDeviceException("Unable to find nbd-client");
    default:
      PLOG(ERROR) << "system returned " << ret_val;
      PLOG(ERROR) << nbd_command_stream.str();
      throw RemoteBlockDeviceException("Hit unexpected switch branch");
  }

  // Unreachable
  return false;
}

void NbdBlockDevice::Disconnect() {
  std::ostringstream nbd_command_stream;

  nbd_command_stream << "nbd-client -d " << block_path_ << " 2>/dev/null 1>&2";

  int ret_val = system(nbd_command_stream.str().c_str());
  if (ret_val == 127) {
    PLOG(ERROR) << "system returned 127";
    throw RemoteBlockDeviceException("Unable to find nbd-client");
  }

  return;
}

NbdBlockDevice::~NbdBlockDevice() {
  try {
    Disconnect();
  } catch (const std::runtime_error &e) {
    LOG(ERROR) << "Error during disconnect in destructor: " << e.what();
  }
}

}
