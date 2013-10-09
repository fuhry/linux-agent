#ifndef DATTO_CLIENT_REQUEST_LISTENER_REQUEST_HANDLER_H_
#define DATTO_CLIENT_REQUEST_LISTENER_REQUEST_HANDLER_H_

// TODO: Include the protobuf class

#include <memory>

#include "request.pb.h"
#include <reply_channel.h>

namespace datto_linux_client {

using namespace std;


class RequestHandler {
 public:

  //  Make non-copyable the C++ 11 way 

  RequestHandler(const RequestHandler&) = delete;
  RequestHandler& operator=(const RequestHandler&) = delete;

  RequestHandler(){};
  // For testing, extend this class and make
  // Handle() do something like print information to stdout
  virtual void Handle(shared_ptr<Request> request,
                      shared_ptr<ReplyChannel> reply_channel) {}
  virtual ~RequestHandler() { }
};
}

#endif //  DATTO_CLIENT_REQUEST_LISTENER_REQUEST_HANDLER_H_
