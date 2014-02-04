#include "block_device/nbd_server.h"

#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "block_device/block_device_exception.h"
#include <glog/logging.h>

namespace datto_linux_client {

NbdServer::NbdServer(const std::string &file_to_serve) {
  uint16_t port_a = rand() % 55535 + 10000;
  Init(file_to_serve, port_a);

}
NbdServer::NbdServer(const std::string &file_to_serve, const uint16_t port_a) {
  Init(file_to_serve, port_a);
}

void NbdServer::Init(const std::string &file_to_serve, const uint16_t port_a) {
  port_ = port_a;

  LOG(INFO) << "Starting nbd-server on " << port_;
  pid_t fork_ret = fork();
  if (fork_ret == -1) {
      PLOG(ERROR) << "fork";
      throw BlockDeviceException("fork failed");
  }
  else if (fork_ret == 0) {
      // This is the same as 2>&1 1>/dev/null
      int null_fd = open("/dev/null", O_RDWR);
      dup2(null_fd, 1);
      dup2(null_fd, 2);
      close(null_fd);
      // -d means it won't daemonize
      // -C /dev/null disables using a configuration file
      execl("/bin/nbd-server", "nbd-server",
            std::to_string(port_).c_str(),
            file_to_serve.c_str(),
            "-d",
            "-C", "/dev/null",
            nullptr);
      // If we get here then exec failed
      PLOG(ERROR) << "execl";
      _exit(127);
  }
  else {
      nbd_server_pid_ = fork_ret;
      // Let the nbd_server start
      usleep(1000000);
  }
}

NbdServer::~NbdServer() {
  LOG(INFO) << "Killing nbd-server";
  if (kill(nbd_server_pid_, SIGTERM)) {
    PLOG(ERROR) << "kill failed, check nbd-server was stopped correctly";
  } else {
    wait(nullptr);
  }
}
}
