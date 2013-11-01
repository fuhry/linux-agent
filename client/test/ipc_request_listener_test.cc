#include "request_listener/ipc_request_listener.h"
#include "request_listener/reply_channel.h"
#include "request_listener/request_handler.h"
#include "request_listener/request_listener_exception.h"

#include "reply.pb.h"
#include "string_reply.pb.h"
#include "request.pb.h"

#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <functional>
#include <chrono>
#include <thread>

#include <gtest/gtest.h>
#include <glog/logging.h>

namespace {
using ::datto_linux_client::IpcRequestListener;
using ::datto_linux_client::Reply;
using ::datto_linux_client::StringReply;
using ::datto_linux_client::ReplyChannel;
using ::datto_linux_client::Request;
using ::datto_linux_client::RequestListenerException;
using ::datto_linux_client::RequestHandler;

// TODO This will fail sometimes for unknown reasons
// Test cases redefine handle_func to set behavior for the RequestHandler
std::function<void(const Request&, std::shared_ptr<ReplyChannel>)> handle_func;

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

  ~RequestTestClient() {
    close(sock_fd_);
  }

  void SendRequest(const Request &request) {
    uint32_t request_size = request.ByteSize();
    request_size = htonl(request_size);

    if (write(sock_fd_, &request_size, sizeof(request_size)) == -1) {
      PLOG(ERROR) << "Error writing to socket fd";
      throw std::runtime_error("Error writing to socket fd");
    }

    if (!request.SerializeToFileDescriptor(sock_fd_)) {
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

    LOG(INFO) << "Getting reply of size: " << reply_size;

    std::string message_buf;
    message_buf.resize(reply_size);

    ssize_t bytes_read = recv(sock_fd_, &message_buf[0], reply_size,
                              MSG_WAITALL);

    if (bytes_read != (int32_t) reply_size) {
      PLOG(ERROR) << "Bytes read: " << bytes_read;
      throw std::runtime_error("Error reading reply");
    }

    Reply reply;
    reply.ParseFromString(message_buf);

    return reply;
  }

 private:
  int sock_fd_;
};

class IpcRequestListenerTest : public ::testing::Test {
 protected:
  IpcRequestListenerTest() {
    handler_ = std::unique_ptr<RequestHandler>(new RequestHandler(nullptr));
    handle_func = [&](const Request&, std::shared_ptr<ReplyChannel>) {
        return;
    };
  }
  std::unique_ptr<RequestHandler> handler_;

};

}

// Define implementation for RequestHandler here
namespace datto_linux_client {

RequestHandler::RequestHandler(std::shared_ptr<BackupManager> bm) { }

void RequestHandler::Handle(const Request &request,
                            std::shared_ptr<ReplyChannel> reply_channel) {
  handle_func(request, reply_channel);
}

}

namespace {

std::string TEST_SOCKET_PATH = "/tmp/test_socket";

TEST_F(IpcRequestListenerTest, Constructor) {
  IpcRequestListener listener(TEST_SOCKET_PATH, std::move(handler_));
}

TEST_F(IpcRequestListenerTest, CallsHandle) {
  std::atomic<bool> handle_called(false);

  handle_func = [&](const Request&, std::shared_ptr<ReplyChannel>) {
    handle_called = true;
  };

  IpcRequestListener listener(TEST_SOCKET_PATH, std::move(handler_));
  RequestTestClient client(TEST_SOCKET_PATH);

  ASSERT_FALSE(handle_called);

  Request req;
  req.set_type(Request::START_BACKUP);

  client.SendRequest(req);

  std::this_thread::yield();
  if (!handle_called) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
  }

  EXPECT_TRUE(handle_called);
}

TEST_F(IpcRequestListenerTest, DoesStop) {
  IpcRequestListener listener(TEST_SOCKET_PATH, std::move(handler_));
  listener.Stop();

  try {
    RequestTestClient client(TEST_SOCKET_PATH);

    Request req;
    req.set_type(Request::START_BACKUP);
    client.SendRequest(req);

    FAIL() << "client succeeded, it should have failed";
  } catch (const std::runtime_error &e) {
    // good
  }
}

TEST_F(IpcRequestListenerTest, SendsReply) {
  IpcRequestListener listener(TEST_SOCKET_PATH, std::move(handler_));

  std::string reply_str = "Test string";

  handle_func = [&](const Request&,
                    std::shared_ptr<ReplyChannel> reply_channel) {

    Reply reply;
    reply.set_type(Reply::STRING);
    reply.mutable_string_reply()->set_message(reply_str);

    ASSERT_EQ(reply_str, reply.string_reply().message());

    reply_channel->SendReply(reply);
  };

  RequestTestClient client(TEST_SOCKET_PATH);

  Request req;
  req.set_type(Request::START_BACKUP);
  client.SendRequest(req);

  Reply reply = client.GetReply();
  EXPECT_EQ(reply_str, reply.string_reply().message());
}

} // namespace
