#ifndef DATTO_CLIENT_MESSAGE_QUEUE_REQUEST_HANDLER_H_
#define DATTO_CLIENT_MESSAGE_QUEUE_REQUEST_HANDLER_H_

// TODO: Include the protobuf class

#include <boost/noncopyable.hpp>
#include <memory>

namespace datto_linux_client {

class RequestHandler : private boost::noncopyable {
 public:
  RequestHandler();
  // For testing, extend this class and make
  // Handle() do something like print information to stdout
  virtual void Handle(std::shared_ptr<Request> request,
                      std::shared_ptr<ReplyChannel> reply_channel);
  virtual ~RequestHandler() { }
};
}

#endif //  DATTO_CLIENT_MESSAGE_QUEUE_REQUEST_HANDLER_H_
