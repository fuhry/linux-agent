#ifndef DATTO_CLIENT_MESSAGE_QUEUE_REPLY_CHANNEL_H_
#define DATTO_CLIENT_MESSAGE_QUEUE_REPLY_CHANNEL_H_

#include <boost/noncopyable.hpp>
#include <memory>

namespace datto_linux_client {

// Inherit from this and make an IpcSocketReplyChannel, which will be
// passed to the ReplyHandler
class ReplyChannel: private boost::noncopyable {
 public:
  virtual void SendReply(std::shared_ptr<Reply> reply);
  virtual bool IsAvailable();
  virtual ~ReplyChannel();
};

}

#endif //  DATTO_CLIENT_MESSAGE_QUEUE_REPLY_CHANNEL_H_