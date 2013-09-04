#ifndef DATTO_CLIENT_MESSAGE_QUEUE_MESSAGE_QUEUE_LISTENER_H_
#define DATTO_CLIENT_MESSAGE_QUEUE_MESSAGE_QUEUE_LISTENER_H_

#include "message_queue/message.h"

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <string>

namespace datto_linux_client {

class MessageQueueListener : private boost::noncopyable {
 public:
  explicit MessageQueueListener(std::string queue_path);
  // Message callback is called for each message
  void Listen(boost::function<void (const Message&)> message_processor);
 private:
  std::string queue_path_;
};
}

#endif //  DATTO_CLIENT_MESSAGE_QUEUE_MESSAGE_QUEUE_LISTENER_H_
