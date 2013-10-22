#include "request_listener/ipc_request_listener.h"
#include "request_listener/request_handler.h"
#include "request_listener/reply_channel.h"

#include "reply.pb.h"
#include "request.pb.h"

#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <functional>

#include <gtest/gtest.h>
#include <glog/logging.h>

namespace {
using ::datto_linux_client::IpcRequestListener;
using ::datto_linux_client::Reply;
using ::datto_linux_client::ReplyChannel;
using ::datto_linux_client::Request;
using ::datto_linux_client::RequestHandler;

// Test cases redefine handle_func to set behavior for the RequestHandler
std::function<void(const Request&,
                   std::shared_ptr<ReplyChannel>)> handle_func;

class RequestTestClient {
 public:
  RequestTestClient(std::string ipc_path) {
    sock_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);

    if (sock_fd_ < 0) {
      PLOG(ERROR) << "Unable to open socket";
      throw std::runtime_error("socket");
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, ipc_path.c_str());

    if (connect(sock_fd_, (struct sockaddr *) &addr, sizeof(addr))) {
      PLOG(ERROR) << "Unable to connect to " << ipc_path;
      throw std::runtime_error("connect");
    }
  }

  void SendRequest(const Request &request) {
    uint32_t request_size = request.ByteSize();
    request_size = htonl(request_size);

    if (write(sock_fd_, &request_size, sizeof(request_size)) == -1) {
      PLOG(ERROR) << "Error writing to socket fd";
      throw std::runtime_error("Error writing to socket fd");
    }

    if (request.SerializeToFileDescriptor(sock_fd_)) {
      PLOG(ERROR) << "Error serializing to fd";
      throw std::runtime_error("Error serializing");
    }
  }

  Reply GetReply() {
    uint32_t reply_size;
    if (read(sock_fd_, &reply_size, sizeof(reply_size)) == -1) {
      PLOG(ERROR) << "read";
      throw std::runtime_error("read");
    }

    reply_size = ntohl(reply_size);

    std::string message_buf;
    message_buf.resize(reply_size);

    ssize_t bytes_read = recv(sock_fd_, &message_buf[0], reply_size, 
                              MSG_WAITALL);

    if (bytes_read != reply_size) {
      PLOG(ERROR) << "Bytes read: " << bytes_read;
      throw std::runtime_error("Error reading reply");
    }

    Reply reply;
    reply.ParseFromString(message_buf);
  }

 private:
  int sock_fd_;
};

class IpcRequestListenerTest : public ::testing::Test {
 protected:
  IpcRequestListenerTest() {
    handle_func = [&](const Request&, std::shared_ptr<ReplyChannel>) {
        return;
    };
  }
};

}

namespace datto_linux_client {


RequestHandler::RequestHandler() { }
RequestHandler::~RequestHandler() { }

void RequestHandler::Handle(const Request &request,
                            std::shared_ptr<ReplyChannel> reply_channel) {
  handle_func(request, reply_channel);
}

}

namespace {

TEST_F(IpcRequestListenerTest, Constructor) {
  std::unique_ptr<RequestHandler> handler(new RequestHandler());
  IpcRequestListener("/tmp/test_socket", std::move(handler));
}

TEST_F(IpcRequestListenerTest, T2) {
  std::unique_ptr<RequestHandler> handler(new RequestHandler());
  IpcRequestListener("/tmp/test_socket", std::move(handler));

  handle_func = [&](const Request &, std::shared_ptr<ReplyChannel>) {
    LOG(ERROR) << "here";
  };

}

TEST_F(IpcRequestListenerTest, T3) {
  std::unique_ptr<RequestHandler> handler(new RequestHandler());
  IpcRequestListener("/tmp/test_socket", std::move(handler));
}

} // namespace
