#include "request_listener/socket_reply_channel.h"
#include <arpa/inet.h>

#include <glog/logging.h>

namespace datto_linux_client {

SocketReplyChannel::SocketReplyChannel(int connection_fd)
    : connection_fd_(connection_fd) { }

void SocketReplyChannel::SendReply(const Reply &reply) {
  uint32_t reply_size = reply.ByteSize();
  DLOG(INFO) << "Sending reply of size " << reply_size;

  reply_size = htonl(reply_size);

  if (write(connection_fd_, &reply_size, sizeof(reply_size)) == -1) {
    PLOG(ERROR) << "Error writing to socket fd";
    throw std::runtime_error("Error writing to socket fd");
  }

  if (!reply.SerializeToFileDescriptor(connection_fd_)) {
    PLOG(ERROR) << "Error on SerializeToFileDescriptor";
    throw RequestListenerException("Error sending over reply channel");
  }
}
}
