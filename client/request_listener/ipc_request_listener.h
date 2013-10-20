#ifndef DATTO_CLIENT_REQUEST_LISTENER_IPC_REQUEST_LISTENER_H_
#define DATTO_CLIENT_REQUEST_LISTENER_IPC_REQUEST_LISTENER_H_

#include <memory>

namespace datto_linux_client {

// RequestListener is responsible for getting backup Requests from 
// a Unix Domain Socket and passing them to the RequestHandler, along with
// a channel for replies
class IpcRequestListener {
 public:
  IpcRequestListener(std::string ipc_socket_path,
                     std::unique_ptr<RequestHandler> request_handler);
  void Stop();

  ~RequestListener();

  IpcRequestListener(const IpcRequestListener&) = delete;
  IpcRequestListener& operator=(const IpcRequestListener&) = delete;

 private:
  int socket_fd;
  std::unique_ptr<RequestHandler> request_handler_;
};
}

#endif //  DATTO_CLIENT_REQUEST_LISTENER_IPC_REQUEST_LISTENER_H_
