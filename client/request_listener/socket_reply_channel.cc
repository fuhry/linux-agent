#include "request_listener/socket_reply_channel.h"

#include <glog/logging.h>

namespace datto_linux_client {

SocketReplyChannel::SocketReplyChannel(int connection_fd)
    : connection_fd_(connection_fd) { }

void SocketReplyChannel::SendReply(const Reply &reply) {
  if (!reply.SerializeToFileDescriptor(connection_fd_)) {
    PLOG(ERROR) << "Error on SerializeToFileDescriptor";
    throw RequestListenerException("Error sending over reply channel");
  }
}
}
