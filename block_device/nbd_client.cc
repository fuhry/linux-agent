#include "block_device/nbd_client.h"

#include <dirent.h>
#include <endian.h>
#include <fcntl.h>
#include <glog/logging.h>
#include <linux/nbd.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "block_device/nbd_exception.h"

namespace {
using ::datto_linux_client::NbdException;

const char NBD_ACK_MAGIC[] = "NBDMAGIC";
const uint64_t NBD_NEGOTIATE_MAGIC = 0x00420281861253LL;

void LoadNBDModule() {
  struct stat buf;
  if (stat("/sys/block/nbd0", &buf) == -1) {
    if (errno == ENOENT) {
      LOG(WARNING) << "NBD isn't compiled in or module isn't loaded";
      LOG(INFO) << "Attempting to load NBD module";
      if (system("modprobe nbd") != 0) {
        LOG(ERROR) << "Unable to load NBD module";
        throw NbdException("Unable to load NBD module.");
      }
    } else {
      PLOG(ERROR) << "Unable to check existance of /sys/block/nbd0";
    }
  }
}

std::string FindOpenNbdDevice() {
  std::string open_device = "";

  DIR *block_dir = opendir("/sys/block");
  if (block_dir == NULL) {
    throw NbdException("Unable to open /sys/block");
  }

  struct dirent *dir_entry;

  while ((dir_entry = readdir(block_dir)) != NULL) {
    if (strncmp(dir_entry->d_name, "nbd", 3) == 0) {
      std::string pid_path = "/sys/block/" + std::string(dir_entry->d_name) +
                             "/pid";
      if (access(pid_path.c_str(), F_OK) == 0) {
        // If the pid file exists, the nbd device is in use
        continue;
      } else {
        if (errno == ENOENT) {
          // ENOENT means the pid file doesn't exist so we can use this one
          open_device = "/dev/" + std::string(dir_entry->d_name);
          break;
        } else {
          // Any other error is a problem
          PLOG(ERROR) << "Error during access";
          closedir(block_dir);
          throw NbdException("Unable to check access of nbd device");
        }
      }
    }
  }
  closedir(block_dir);
  int error = errno;

  if (open_device == "") {
    if (error) {
      PLOG(ERROR) << "Error during readdir";
      throw NbdException("Error while reading /sys/block/*");
    } else {
      throw NbdException("Unable to find open nbd device");
    }
  }

  return open_device;
}

int OpenNbdSocket(std::string host, uint16_t port) {
  int sock;

  // see man 3 getaddrinfo
  struct addrinfo hints = {};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_ADDRCONFIG;
  hints.ai_protocol = IPPROTO_TCP;

  struct addrinfo *ai;
  int err = getaddrinfo(host.c_str(), std::to_string(port).c_str(),
                        &hints, &ai);
  if (err) {
    throw NbdException("getaddinfo error: " + std::string(gai_strerror(err)));
  }

  bool success = false;
  for (struct addrinfo *ai_iter = ai;
       ai_iter != NULL;
       ai_iter = ai_iter->ai_next) {
    sock = socket(ai_iter->ai_family, ai_iter->ai_socktype,
                  ai_iter->ai_protocol);

    if (sock == -1) {
      continue;
    }

    if (connect(sock, ai_iter->ai_addr, ai_iter->ai_addrlen) != -1) {
      success = true;
      break;
    }
  }

  freeaddrinfo(ai);

  if (!success) {
    throw NbdException("Unable to open socket");
  }

  DLOG(INFO) << "Opened NBD socket";

  return sock;
}

// Header from server should be:
// 8 bytes:   first magic
// 8 bytes:   second (version based??) magic
// 8 bytes:   size of remote block device
// 4 bytes:   block device flags
// 124 bytes: zeros
void NegotiateNbdCommunication(int sock, uint64_t *block_device_size) {
  // 9 0s so we can use strcmp below without issue
  char buf[256] = "\0\0\0\0\0\0\0\0\0";
  uint64_t second_magic;

  if (read(sock, buf, 8) < 0) {
    PLOG(ERROR) << "Read magic 1 from sock";
    throw NbdException("Unable to read first magic from socket");
  }

  if (strlen(buf) == 0) {
    throw NbdException("Server closed connection before sending data");
  }

  if (strcmp(buf, NBD_ACK_MAGIC)) {
    LOG(ERROR) << "Got bad first NBD magic number 0x"
               << std::hex << (uint64_t)buf << std::dec;
    throw NbdException("Got bad first magic number");
  }

  if (read(sock, &second_magic, sizeof(second_magic)) < 0) {
    PLOG(ERROR) << "Read magic 2 from sock";
    throw NbdException("Unable to read second magic from socket");
  }
  second_magic = be64toh(second_magic);

  if (second_magic != NBD_NEGOTIATE_MAGIC) {
    LOG(ERROR) << "Got bad second NBD magic number 0x"
               << std::hex << second_magic << std::dec;
    throw NbdException("Got bad second magic number");
  }

  if (read(sock, block_device_size, sizeof(*block_device_size)) < 0) {
    PLOG(ERROR) << "Read block size from sock";
    throw NbdException("Unable to read block size from socket");
  }

  *block_device_size = be64toh(*block_device_size);

  // The server sends 124 bytes of zeros after the 4 byte flag argument.
  // We skip the flag argument as not all kernels support it, so read 128.
  if (read(sock, &buf, 128) < 0) {
    PLOG(ERROR) << "Read negotiation zeros from sock";
    throw NbdException("Unable to read final negotiation zeros from socket");
  }
}

} // unnamed namespace

namespace datto_linux_client {

NbdClient::NbdClient(std::string a_host, uint16_t a_port)
    : host_(a_host),
      port_(a_port),
      disconnect_(false) {

  LoadNBDModule();

  sock_ = OpenNbdSocket(host_, port_);
  NegotiateNbdCommunication(sock_, &block_device_size_);

  nbd_device_path_ = FindOpenNbdDevice();
  LOG(INFO) << "Found open NBD device " << nbd_device_path_;

  nbd_fd_ = open(nbd_device_path_.c_str(), O_RDWR);
  if (nbd_fd_ < 0) {
    throw NbdException("Unable to open NBD device");
  }

  ConfigureNbdDevice();

  nbd_do_it_thread_ = std::thread([&](){
    // This ioctl blocks indefinitely when things are working properly.
    // This means if ever return then there was an error
    int ret = ioctl(nbd_fd_, NBD_DO_IT);
    int error = errno;

    if (ret) {
      if (error == EBADR) {
        // Only show an error if we didn't trigger the disconnect
        if (!disconnect_) {
          LOG(ERROR) << "Disconnected from external source";
        }
      } else {
        PLOG(ERROR) << "NBD_DO_IT ioctl returned with " << ret;
      }
    }
    return;
  });

  // Give the ioctl a chance to run
  std::this_thread::yield();
}

void NbdClient::ConfigureNbdDevice() {
  if (ioctl(nbd_fd_, NBD_SET_BLKSIZE, 4096UL) < 0) {
    PLOG(ERROR) << "NBD_SET_BLKSIZE";
    throw NbdException("Unable to set block size for NBD device");
  }

  // Shifting by 12 (2^12 = 4096) is the way the original nbd code does it
  if (ioctl(nbd_fd_, NBD_SET_SIZE_BLOCKS, block_device_size_ >> 12) < 0) {
    PLOG(ERROR) << "NBD_SET_SIZE_BLOCKS";
    throw NbdException("Unable to set block size for NBD device");
  }

  // Don't check these as we don't care if there was no socket
  ioctl(nbd_fd_, NBD_CLEAR_SOCK);

  // we use unsigned long because that's how it's defined in the kernel
  // function __nbd_ioctl
  if (ioctl(nbd_fd_, NBD_SET_TIMEOUT, 10UL)) {
    PLOG(ERROR) << "NBD_SET_TIMEOUT";
    throw NbdException("Unable to set timeout for NBD device");
  }

  if (ioctl(nbd_fd_, NBD_SET_SOCK, sock_) < 0) {
    PLOG(ERROR) << "NBD_SET_SOCK";
    throw NbdException("Unable to set socket for NBD device");
  }
}

bool NbdClient::IsConnected() {
  int last_slash_pos = nbd_device_path_.rfind('/');
  std::string block_device_name = nbd_device_path_.substr(last_slash_pos + 1);

  std::string pid_path = "/sys/block/" + block_device_name + "/pid";

  LOG(INFO) << "Checking if " << pid_path << " exists for NBD connectivity";

  if (access(pid_path.c_str(), F_OK)) {
    if (errno == ENOENT) {
      return false;
    } else {
      PLOG(ERROR) << "access threw an unexpected error";
      return false;
    }
  }
  return true;
}

void NbdClient::Disconnect() {
  if (!disconnect_) {
    LOG(INFO) << "Disconnecting NbdClient";
    disconnect_ = true;
    if (ioctl(nbd_fd_, NBD_DISCONNECT) < 0) {
      if (errno != EINVAL) {
        PLOG(ERROR) << "NBD_DISCONNECT";
      } else {
        PLOG(ERROR) << "EINVAL";
      }
    }

    if (ioctl(nbd_fd_, NBD_CLEAR_QUE) < 0) {
      PLOG(ERROR) << "NBD_CLEAR_QUE";
    }

    if (ioctl(nbd_fd_, NBD_CLEAR_SOCK) < 0) {
      PLOG(ERROR) << "NBD_CLEAR_SOCK";
    }

    nbd_do_it_thread_.join();
  }
}

NbdClient::~NbdClient() {
  try {
    Disconnect();
  } catch (const std::runtime_error &e) {
    LOG(ERROR) << e.what();
  }
  close(nbd_fd_);
  close(sock_);
}

}
