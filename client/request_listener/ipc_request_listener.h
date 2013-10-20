#ifndef DATTO_CLIENT_REQUEST_LISTENER_IPC_REQUEST_LISTENER_H_
#define DATTO_CLIENT_REQUEST_LISTENER_IPC_REQUEST_LISTENER_H_

#include <memory>
#include <stdexcept>
#include <string>

#include <protosocket.h>
#include <test_request_handler.h>
#include <ipc_socket_reply_channel.h>


using namespace std;
namespace datto_linux_client {

class IpcSocketRequestListenerException : public runtime_error {
  public :
    explicit IpcSocketRequestListenerException(string &what) : runtime_error(what) {}
};


  using namespace std;

// RequestListener is responsible for getting backup Requests from 
// a Unix Domain Socket and passing them to the RequestHandler, along with
// a channel for replies.
class IpcSocketRequestListener  {

 private:
  shared_ptr<ProtoSocket> ps_;    // pointer to listener protosocket
  shared_ptr<RequestHandler> rh_;

 public:

  //  Make non-copyable ala C++ 11

  IpcSocketRequestListener(const IpcSocketRequestListener &) = delete;
  IpcSocketRequestListener & operator=(const IpcSocketRequestListener &) = delete;

  //  Constructor, opens a UNIX socket for listening

  IpcSocketRequestListener(string ipc_socket_path,           
                           shared_ptr<RequestHandler> request_handler);
  
  //  Constructor, opens a TCP socket for listening

  IpcSocketRequestListener(int32_t  ipc_socket_port,
                           shared_ptr<RequestHandler> request_handler);

  //  Start a listen loop.  Never returns.  performs the functions of the GetNewConnection()
  //  and Handle() methods repeatedly, ad naseum.

  void Listen();

  //  Get a single new connection.  Waits wait_time milliseconds for a new connection,
  //  and returns nullptr if no new connection is made

  shared_ptr<ProtoSocket> GetNewConnection(int wait_time);

  //  As above, but waits indefinitely for a new connection

  shared_ptr<ProtoSocket> GetNewConnection();

  //  Invokes the Handle method of the request handler.  Responsible for grabbing
  //  the request, converting it from an array of bytes to a Request protobuffer, 
  //  creating a ReplyChannel object, and making the call to the Handler method.

  int Handle(shared_ptr<ProtoSocket> conn_proto_socket);

  //  Destructor, closes socket contained within socket_fd_
  ~IpcSocketRequestListener();

};   // end of class definition
}   // end of namespace

#endif //  DATTO_CLIENT_REQUEST_LISTENER_IPC_REQUEST_LISTENER_H_
