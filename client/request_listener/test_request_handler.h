#ifndef DATTO_CLIENT_TEST_REQUEST_LISTENER_TEST_REQUEST_HANDLER_H_
#define DATTO_CLIENT_TEST_REQUEST_LISTENER_TEST_REQUEST_HANDLER_H_

// TODO: Include the protobuf class

#include <memory>
#include <stdexcept>

#include <protobuf_classes/request.pb.h>
#include <reply_channel.h>
#include <request_handler.h>

namespace datto_linux_client {

using namespace std;

// Exception class for TestRequestHandler

class TestRequestHandlerException : public runtime_error {
  public:
    explicit TestRequestHandlerException(string &what) : runtime_error(what){};
};
  

// This class is derived from RequestHandler for testing purposes
class TestRequestHandler : public RequestHandler {

  private:

    void handle_start(shared_ptr<Request> request,
                      shared_ptr<ReplyChannel> reply_channel);

    void handle_stop(shared_ptr<Request> request,
                     shared_ptr<ReplyChannel> reply_channel);
  
    void handle_pause(shared_ptr<Request> request,
                      shared_ptr<ReplyChannel> reply_channel);
  
    void handle_status(shared_ptr<Request> request,
                       shared_ptr<ReplyChannel> reply_channel);
  
  public:

  //  Make non-copyable the C++ 11 way 

    TestRequestHandler(const TestRequestHandler&) = delete;
    TestRequestHandler& operator=(const TestRequestHandler&) = delete;

    TestRequestHandler();

    virtual void Handle(shared_ptr<Request> request,
                      shared_ptr<ReplyChannel> reply_channel);

    virtual ~TestRequestHandler(); 

};

}

#endif //  DATTO_CLIENT_TEST_REQUEST_LISTENER_TEST_REQUEST_HANDLER_H_
