#include "remote_block_device/nbd_block_device.h"
#include "remote_block_device/remote_block_device_exception.h"

#include <glog/logging.h>
#include <sstream>

namespace datto_linux_client {

// TODO: If this ends up making it to prod, we should stop using system
// and start using exec(). As I *think* this is temporary, don't worry about
// this yet.
NbdBlockDevice::NbdBlockDevice(std::string remote_host, uint16_t remote_port,
                               std::string local_block_path) {

  std::ostringstream nbd_command_stream;

  nbd_command_stream << "nbd-client " << remote_host << " " << remote_port
                     << " " << local_block_path << " 2>/dev/null 1>&2";

  int ret_val = system(nbd_command_stream.str().c_str());

  switch (ret_val) {
    case 0:
      // success case
      break;
    case -1:
      // system() error case
      PLOG(ERROR) << nbd_command_stream.str() << " :  system returned -1";
      throw RemoteBlockDeviceException("Unable to mount " + local_block_path);
      break;
    case 127:
      // command not found case
      LOG(ERROR) << "'" << nbd_command_stream.str() << "' not found";
      throw RemoteBlockDeviceException("Unable to mount " + local_block_path);
      break;
    default:
      // nbd-client error
      PLOG(ERROR) << nbd_command_stream.str() << " :  nbd-client returned "
                  << ret_val;
      throw RemoteBlockDeviceException("Unable to mount " + local_block_path);
      break;
  }

  block_path_ = local_block_path;
  // Setup major_, minor_, etc
  BlockDevice::Init();
}

bool NbdBlockDevice::IsConnected() const {
  std::ostringstream nbd_command_stream;

  nbd_command_stream << "nbd-client -c " << block_path_
                     << " 2>/dev/null 1>&2";
  int ret_val = system(nbd_command_stream.str().c_str());

  switch (ret_val) {
    case 0:
      return true;
      break;
    case 1:
      return false;
      break;
    case 2:
      throw RemoteBlockDeviceException("Error while checking nbd connection");
      break;
    case 127:
      PLOG(ERROR) << "system returned 127";
      throw RemoteBlockDeviceException("Unable to find nbd-client");
      break;
    default:
      throw RemoteBlockDeviceException("Hit unexpected switch branch");
      break;
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
  } catch (...) {
    LOG(ERROR) << "Error during disconnect in destructor";
  }
}

}
