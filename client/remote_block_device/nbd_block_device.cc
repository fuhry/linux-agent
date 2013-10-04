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
NbdBlockDevice::NbdBlockDevice(std::string remote_host, uint16_t remote_port,
                               std::string local_block_path) {

  pid_t fork_ret = fork();

  if (fork_ret == 0) {
    int null_fd = open("/dev/null", O_RDWR);
    dup2(null_fd, 0);
    dup2(null_fd, 1);
    dup2(null_fd, 2);
    close(null_fd);
    execlp("nbd-client", "nbd-client", "-n", remote_host.c_str(),
           std::to_string(remote_port).c_str(),
           local_block_path.c_str(), nullptr);
    PLOG(ERROR) << "execlp returned!";
    _exit(127);
  } else if (fork_ret < 0) {
    PLOG(ERROR) << "fork failed";
    throw BlockDeviceException("fork");
  }

  nbd_client_pid_ = fork_ret;
  block_path_ = local_block_path;

  // Let the client start before continuing
  sleep(2);

  int status;
  pid_t result = waitpid(nbd_client_pid_, &status, WNOHANG);

  if (result == -1) { // error from waitpid()
    PLOG(ERROR) << "waitpid";
    throw BlockDeviceException("waitpid couldn't check child");
  } else if (result != 0) { // child isn't running
    if (WIFEXITED(status)) {
      LOG(ERROR) << "client returned with status of " << WEXITSTATUS(status);
      throw BlockDeviceException("child terminated by exit()");
    } else if (WIFSIGNALED(status)) {
      LOG(ERROR) << "client received signal number " << WTERMSIG(status);
      throw BlockDeviceException("child terminated by signal");
    } else {
      PLOG(ERROR) << "client died for unknown reason";
      throw BlockDeviceException("child terminated by unknown");
    }
  }

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
      break;
    case 1:
      return false;
      break;
    case 2:
      PLOG(ERROR) << nbd_command_stream.str();
      throw RemoteBlockDeviceException("Error while checking nbd connection");
      break;
    case 127:
      PLOG(ERROR) << "system returned 127";
      PLOG(ERROR) << nbd_command_stream.str();
      throw RemoteBlockDeviceException("Unable to find nbd-client");
      break;
    default:
      PLOG(ERROR) << "system returned " << ret_val;
      PLOG(ERROR) << nbd_command_stream.str();
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
