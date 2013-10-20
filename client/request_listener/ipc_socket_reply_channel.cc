#include <memory>

#include <protosocket.h>
#include <ipc_socket_reply_channel.h>
#include <iostream>

namespace datto_linux_client {

  using namespace std;

  bool IPCSocketReplyChannel::IsAvailable() {

    return true;
  }

  void IPCSocketReplyChannel::SendReply(shared_ptr<Reply> reply) {

    int reply_size = reply->ByteSize();

    unsigned char *serialized_reply = (unsigned char *)malloc(reply_size);

    reply->SerializeToArray(serialized_reply, reply_size);

    ps_->SendMessage(reply_size, serialized_reply);

  }
   
  IPCSocketReplyChannel::~IPCSocketReplyChannel() {

  }

}


