#ifndef DATTO_CLIENT_REQUEST_LISTENER_IPC_SOCKET_REPLY_CHANNEL_H_
#define DATTO_CLIENT_REQUEST_LISTENER_IPC_SOCKET_REPLY_CHANNEL_H_

#include <memory>

#include <reply_channel.h>
#include "reply.pb.h"
#include <protosocket.h>

namespace datto_linux_client {

using namespace std;

class IPCSocketReplyChannel: public ReplyChannel {

  private: 
    shared_ptr<ProtoSocket> ps_;

  public:

  //  Make non-copyable ala C++ 11

    IPCSocketReplyChannel(const IPCSocketReplyChannel&) = delete;
    IPCSocketReplyChannel & operator=(const IPCSocketReplyChannel &) = delete;

    inline IPCSocketReplyChannel() : ps_(NULL) {};
    inline IPCSocketReplyChannel(shared_ptr<ProtoSocket> ps) : ps_(ps) {};

    virtual void SendReply(std::shared_ptr<Reply> reply);
    virtual bool IsAvailable();
    virtual ~IPCSocketReplyChannel();
};

}

#endif //  DATTO_CLIENT_REQUEST_LISTENER_IPC_SOCKET_REPLY_CHANNEL_H_

