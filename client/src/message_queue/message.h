#ifndef DATTO_CLIENT_MESSAGE_QUEUE_MESSAGE_QUEUE_LISTENER_H_
#define DATTO_CLIENT_MESSAGE_QUEUE_MESSAGE_QUEUE_LISTENER_H_

#include <string>

namespace datto_linux_client {

class Message {
 public:
  explicit Message(char *message, int message_length);
  MessageType message_type();
 private:
  MessageType message_type_;
};

enum MessageType {
  ECHO = 'E',
  FULL = 'F',
  INCREMENTAL = 'I',
  STOP = 'S',
};

}

#endif //  DATTO_CLIENT_MESSAGE_QUEUE_MESSAGE_QUEUE_LISTENER_H_
