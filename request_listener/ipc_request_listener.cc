#include "request_listener/ipc_request_listener.h"

#include <glog/logging.h>
#include <chrono>
#include <thread>
#include <vector>

#include <arpa/inet.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include "request_listener/request_listener_exception.h"
#include "request_listener/socket_reply_channel.h"

#include "request.pb.h"

namespace {

using datto_linux_client::RequestListenerException;
using datto_linux_client::Request;

const int SOCKET_BACKLOG = 5;

int OpenSocket(std::string ipc_socket_path) {
  LOG(INFO) << "Opening socket " << ipc_socket_path;
  const char *ipc_path_cstr = ipc_socket_path.c_str();

  int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);

  if (sock_fd < 0) {
    PLOG(ERROR) << "Unable to open socket";
    throw RequestListenerException("Unable to open socket");
  }

  if (unlink(ipc_path_cstr)) {
    if (errno != ENOENT) {
      PLOG(ERROR) << "Error removing " << ipc_path_cstr;
      throw RequestListenerException("Unable to remove socket path");
    }
  }

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, ipc_path_cstr, ipc_socket_path.length());

  if (bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr))) {
    PLOG(ERROR) << "Error during bind";
    throw RequestListenerException("Unable to bind socket");
  }

  if (listen(sock_fd, SOCKET_BACKLOG)) {
    PLOG(ERROR) << "Unable to listen on socket";
    throw RequestListenerException("Unable to listen on socket");
  }

  LOG(INFO) << "Listening on socket " << ipc_socket_path;

  return sock_fd;
}

bool ConnectionIsReady(int socket_fd, int timeout_millis) {
  struct pollfd pfd;
  pfd.fd = socket_fd;
  pfd.events = POLLIN;
  switch (poll(&pfd, 1, timeout_millis)) {
    case -1:
      PLOG(ERROR) << "Unable to poll file descriptor";
      throw RequestListenerException("Poll failure");
    case 0:
      return false;
    case 1:
      return true;
    default:
      throw RequestListenerException("Unexpected branch");
  }
}

int GetNewConnection(int socket_fd) {
  int connection_fd = accept(socket_fd, NULL, NULL);
  if (connection_fd < 0) {
    PLOG(ERROR) << "Error accepting connection";
    throw RequestListenerException("Unable to accept connection");
  }

  return connection_fd;
}

Request ReadProtobufRequest(int connection_fd) {
  uint32_t message_length;
  ssize_t bytes_read;

  bytes_read = read(connection_fd, &message_length, sizeof(uint32_t));

  if (bytes_read != sizeof(uint32_t)) {
    PLOG(ERROR) << "Unable to read message length from socket";
    throw RequestListenerException("Error during read from socket");
  }

  message_length = ntohl(message_length);
  LOG(INFO) << "Getting request of size: " << message_length;

  // Sanity check, shouldn't be larger than 1MB
  if (message_length > 1 * 1024 * 1024) {
    LOG(ERROR) << "Message length: " << message_length;
    throw RequestListenerException("Message was too big");
  }

  std::string message_buf;
  message_buf.resize(message_length);

  bytes_read = recv(connection_fd, &message_buf[0], message_length,
                    MSG_WAITALL);

  if (bytes_read != (int32_t)message_length) {
    PLOG(ERROR) << "Bytes read: " << bytes_read;
    throw RequestListenerException("Error reading request");
  }

  Request request;
  request.ParseFromString(message_buf);

  return request;
}

} // namespace

namespace datto_linux_client {

IpcRequestListener::IpcRequestListener(
    std::string ipc_socket_path,
    std::unique_ptr<RequestHandler> request_handler)
    : request_handler_(std::move(request_handler)),
      do_stop_(false) {

  socket_fd_ = OpenSocket(ipc_socket_path);

  std::atomic<bool> started(false);

  LOG(INFO) << "Starting IPC listener thread";
  listen_thread_ = std::thread([&]() {
    while (!do_stop_) {
      try {
        if (ConnectionIsReady(socket_fd_, 500)) {
          if (do_stop_) {
            break;
          }
          int connection_fd = GetNewConnection(socket_fd_);

          // The reply_channel is responsible for cleaning up the
          // connection_fd, so we construct it as soon as possible
          // in case there is an exception.
          std::shared_ptr<ReplyChannel> reply_channel(
              new SocketReplyChannel(connection_fd));

          auto request = ReadProtobufRequest(connection_fd);

          request_handler_->Handle(request, reply_channel);
        }
      } catch (const std::exception &e) {
        LOG(ERROR) << "Exception during connection loop: " << e.what();
      }
      started = true;
    }
    LOG(INFO) << "IPC listener thread stopped";
  });

  while (!started) {
    std::this_thread::yield();
  }
}

void IpcRequestListener::Stop() {
  do_stop_ = true;
  shutdown(socket_fd_, SHUT_RDWR);
  close(socket_fd_);
}

IpcRequestListener::~IpcRequestListener() {
  LOG(INFO) << "Tearing down IpcRequestListener";
  Stop();
  listen_thread_.join();
}

}
