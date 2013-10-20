#ifndef DATTO_CLIENT_REQUEST_LISTENER_REQUEST_HANDLER_H_
#define DATTO_CLIENT_REQUEST_LISTENER_REQUEST_HANDLER_H_

#include <memory>

#include "request_listener/reply_channel.h"
#include "request_listener/protobuf_classes/request.pb.h"

namespace datto_linux_client {
class RequestHandler {
 public:
  RequestHandler(const RequestHandler&) = delete;
  RequestHandler& operator=(const RequestHandler&) = delete;

  RequestHandler() { }
  void Handle(const Request &request,
              std::shared_ptr<ReplyChannel> reply_channel);
  ~RequestHandler() { }
};
}

#endif //  DATTO_CLIENT_REQUEST_LISTENER_REQUEST_HANDLER_H_
