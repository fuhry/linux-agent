#ifndef DATTO_CLIENT_REQUEST_LISTENER_IPC_REQUEST_LISTENER_H_
#define DATTO_CLIENT_REQUEST_LISTENER_IPC_REQUEST_LISTENER_H_

#include "request_listener/request_handler.h"

#include <atomic>
#include <memory>
#include <thread>

namespace datto_linux_client {

// RequestListener is responsible for getting backup Requests from 
// a Unix Domain Socket and passing them to the RequestHandler, along with
// a channel for replies
class IpcRequestListener {
 public:
  IpcRequestListener(std::string ipc_socket_path,
                     std::unique_ptr<RequestHandler> request_handler);
  ~IpcRequestListener();

  void Stop();

  IpcRequestListener(const IpcRequestListener&) = delete;
  IpcRequestListener& operator=(const IpcRequestListener&) = delete;

 private:
  int socket_fd_;
  std::unique_ptr<RequestHandler> request_handler_;
  std::atomic<bool> do_stop_;

  std::thread listen_thread_;
};
}

#endif //  DATTO_CLIENT_REQUEST_LISTENER_IPC_REQUEST_LISTENER_H_
