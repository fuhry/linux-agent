#ifndef DATTO_CLIENT_REQUEST_LISTENER_REPLY_CHANNEL_H_
#define DATTO_CLIENT_REQUEST_LISTENER_REPLY_CHANNEL_H_

#include <boost/noncopyable.hpp>
#include <memory>

namespace datto_linux_client {

// Inherit from this and make an IpcSocketReplyChannel, which will be
// passed to the ReplyHandler
class ReplyChannel: private boost::noncopyable {
 public:
  virtual void SendReply(std::shared_ptr<Reply> reply) = 0;
  virtual bool IsAvailable() = 0;
  virtual ~ReplyChannel() { }
};

}

#endif //  DATTO_CLIENT_REQUEST_LISTENER_REPLY_CHANNEL_H_
