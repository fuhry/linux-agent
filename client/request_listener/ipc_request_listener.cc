#include "request_listener/ipc_request_listener.h"

#include <glog/logging.h>
#include <vector>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include "request_listener/request_listener_exception.h"
#include "request_listener/socket_reply_channel.h"

#include "request.pb.h"

namespace {

using ::datto_linux_client::RequestListenerException;
using ::datto_linux_client::Request;

const int SOCKET_BACKLOG = 5;

int open_socket(std::string ipc_socket_path) {
  int sock_fd;
  const char *ipc_path_cstr = ipc_socket_path.c_str();

  sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);

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

  return sock_fd;
}

// TODO Timeout
int get_new_connection(int socket_fd) {
  int connection_fd = accept(socket_fd, NULL, NULL);
  if (connection_fd < 0) {
    PLOG(ERROR) << "Error accepting connection";
    throw RequestListenerException("Unable to accept connection");
  }

  return connection_fd;
}

// TODO Determine if copy constructor here is okay
//      during diff backups
Request read_protobuf_request(int connection_fd) {
  uint32_t message_length;
  ssize_t bytes_read;
  
  bytes_read = read(connection_fd, &message_length, sizeof(uint32_t));

  if (bytes_read != sizeof(uint32_t)) {
    PLOG(ERROR) << "Unable to read message length from socket";
    throw RequestListenerException("Error during read from socket");
  }

  message_length = ntohl(message_length);

  // Sanity check, shouldn't be larger than 1MB
  if (message_length < 1 * 1024 * 1024) {
    LOG(ERROR) << "Message length: " << message_length;
    throw RequestListenerException("Message was too big");
  }
  
  std::string message_buf;
  message_buf.resize(message_length);

  bytes_read = recv(connection_fd, &message_buf[0], message_length, 
                    MSG_WAITALL);

  if (bytes_read != message_length) {
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

  socket_fd_ = open_socket(ipc_socket_path);

  listen_thread_ = std::thread([&]() {
    while (!do_stop_) {
      try {
        int connection_fd = get_new_connection(socket_fd_);
        auto request = read_protobuf_request(connection_fd);
        
        std::shared_ptr<ReplyChannel> reply_channel(
            new SocketReplyChannel(connection_fd));

        request_handler_->Handle(request, reply_channel);
      } catch (const std::runtime_error &e) {
        LOG(ERROR) << e.what();
      }
    }
  });
}

void IpcRequestListener::Stop() {
  do_stop_ = true;
}

IpcRequestListener::~IpcRequestListener() {
  close(socket_fd_);
  Stop();
  listen_thread_.join();
}

}
