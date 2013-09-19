#ifndef DATTO_CLIENT_MESSAGE_QUEUE_IPC_REQUEST_LISTENER_H_
#define DATTO_CLIENT_MESSAGE_QUEUE_IPC_REQUEST_LISTENER_H_

#include <boost/noncopyable.hpp>
#include <memory>

namespace datto_linux_client {

// RequestListener is responsible for getting backup Requests from 
// a Unix Domain Socket and passing them to the RequestHandler, along with
// a channel for replies.
class IpcSocketRequestListener : private boost::noncopyable {
 public:
  RequestListener(std::string ipc_socket_path,
                  std::unique_ptr<RequestHandler> request_handler);
  ~RequestListener();

 private:
  int socket_fd;
  std::unique_ptr<RequestHandler> request_handler_;
};
}

#endif //  DATTO_CLIENT_MESSAGE_QUEUE_IPC_REQUEST_LISTENER_H_
