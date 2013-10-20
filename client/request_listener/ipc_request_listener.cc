#include "request_listener/ipc_request_listener.h"

namespace datto_linux_client {

IpcRequestListener::IpcRequestListener(
    std::string ipc_socket_path,
    std::unique_ptr<RequestHandler> request_handler) {

}

}
