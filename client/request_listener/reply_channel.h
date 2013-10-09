#ifndef DATTO_CLIENT_REQUEST_LISTENER_REPLY_CHANNEL_H_
#define DATTO_CLIENT_REQUEST_LISTENER_REPLY_CHANNEL_H_

#include <memory>

#include <protobuf_classes/reply.pb.h>

namespace datto_linux_client {

// Inherit from this and make an IpcSocketReplyChannel, which will be
// passed to the ReplyHandler
class ReplyChannel {
 public:
   // Make non-copyable the C++ 11 way

  ReplyChannel(const ReplyChannel&) = delete;
  ReplyChannel & operator=(const ReplyChannel &) = delete;

  ReplyChannel() {}
  virtual void SendReply(std::shared_ptr<Reply> reply) = 0;
  virtual bool IsAvailable() = 0;
  virtual ~ReplyChannel() { }
};

}

#endif //  DATTO_CLIENT_REQUEST_LISTENER_REPLY_CHANNEL_H_
