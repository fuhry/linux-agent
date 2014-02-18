#ifndef DATTO_CLIENT_REQUEST_LISTENER_REPLY_CHANNEL_H_
#define DATTO_CLIENT_REQUEST_LISTENER_REPLY_CHANNEL_H_

#include <memory>

#include "reply.pb.h"

namespace datto_linux_client {

class ReplyChannel {
 public:
  ReplyChannel() { }

  virtual void SendReply(const Reply &reply) = 0;
  virtual bool IsAvailable() = 0;
  virtual ~ReplyChannel() { }

  ReplyChannel(const ReplyChannel&) = delete;
  ReplyChannel& operator=(const ReplyChannel&) = delete;

};

}

#endif //  DATTO_CLIENT_REQUEST_LISTENER_REPLY_CHANNEL_H_
