#ifndef DATTO_CLIENT_REQUEST_LISTENER_SOCKET_REPLY_CHANNEL_H_
#define DATTO_CLIENT_REQUEST_LISTENER_SOCKET_REPLY_CHANNEL_H_

#include "request_listener/reply_channel.h"
#include "request_listener/request_listener_exception.h"

#include "reply.pb.h"

#include <memory>

namespace datto_linux_client {

class SocketReplyChannel : public ReplyChannel {
 public:
  SocketReplyChannel(int connection_fd);

  virtual void SendReply(const Reply &reply);
  virtual bool IsAvailable() { throw std::runtime_error("not implemented"); }
  virtual ~SocketReplyChannel() { }

  SocketReplyChannel(const SocketReplyChannel&) = delete;
  SocketReplyChannel& operator=(const SocketReplyChannel&) = delete;

 private:
  int connection_fd_;
};

}

#endif //  DATTO_CLIENT_REQUEST_LISTENER_SOCKET_REPLY_CHANNEL_H_
