#include <stdexcept>
#include <memory>
#include <protosocket.h>
#include <ipc_request_listener.h>

namespace datto_linux_client {

  using namespace std;

// RequestListener is responsible for getting backup Requests from 
// a Unix Domain Socket and passing them to the RequestHandler, along with
// a channel for replies.

  //  Constructor, opens a UNIX socket for listening

  IpcSocketRequestListener::IpcSocketRequestListener(
      string ipc_socket_path,           
      shared_ptr<RequestHandler> request_handler) : rh_(request_handler) {

    ProtoSocket *tps = new ProtoSocket;   // Get a new ProtoSocket object
    ps_.reset(tps);  

    try {
      ps_->OpenUNIXListener(ipc_socket_path);    // Open for listening (UNIX socket)
    }
    catch (ProtoSocketException &e) {
      string err = "Error opening UNIX socket for listening: ";
      err += e.what();
      throw IpcSocketRequestListenerException(err);
    }


  }
  
  //  Constructor, opens a TCP socket for listening

  IpcSocketRequestListener::IpcSocketRequestListener(
      int32_t  ipc_socket_port,
      shared_ptr<RequestHandler> request_handler) : rh_(request_handler) {

    ProtoSocket *tps = new ProtoSocket;   // Get a new ProtoSocket object
    ps_.reset(tps);  

    try {
      ps_->OpenTCPListener(ipc_socket_port);   // Open for listening  (TCP/IP socket)
    }
    catch (ProtoSocketException &e) {
      string err = "Error opening TCP/IP socket for listening: ";
      err += e.what();
      throw IpcSocketRequestListenerException(err);
    }

  }

  //  Start a listen loop.  Never returns.  performs the functions of the GetNewConnection()
  //  and Handle() methods repeatedly, ad naseum.

  void IpcSocketRequestListener::Listen() {

    for(;;) {

      shared_ptr<ProtoSocket> ps = GetNewConnection();  //  Wait forever for a new connection
      Handle(ps);    // Handle the message
      ps.reset();

    }

  }


  //  Get a single new connection.  Waits wait_time milliseconds for a new connection,
  //  and returns nullptr if no new connection is made

  shared_ptr<ProtoSocket> IpcSocketRequestListener::GetNewConnection(int wait_time) {

    if ( ps_->CanRead(wait_time) ) {     //  Wait for a connection to be available
      unique_ptr<ProtoSocket> utps = ps_->NewConnection();    // Create new ProtoSocket object with connection
      ProtoSocket *tps = utps.release();
      shared_ptr<ProtoSocket> stps;
      stps.reset(tps);
      return stps;
    }
  }

  //  As above, but waits indefinitely for a new connection

  shared_ptr<ProtoSocket> IpcSocketRequestListener::GetNewConnection() {
    return GetNewConnection(-1);
  }

  //  Invokes the Handle method of the request handler.  Responsible for grabbing
  //  the request, converting it from an array of bytes to a Request protobuffer, 
  //  creating a ReplyChannel object, and making the call to the Handler method.

  int IpcSocketRequestListener::Handle(shared_ptr<ProtoSocket> conn_proto_socket) {

    unique_ptr<ProtoSocketMessage> psm;
    try {    //  Read a message; pass exception up if an error occurs
      psm = conn_proto_socket->GetMessage();
    }
    catch (ProtoSocketException &e) {
      string err("Error reading from socket: ");
      err += e.what();
      throw IpcSocketRequestListenerException(err);
    }

    // shared_ptr<Request> req = make_shared<Request>;    //  Parse the data into a Request object
    Request * treq = new Request;    //  Parse the data into a Request object
    treq->ParseFromArray((psm->buffer).get(), psm->size);
    shared_ptr<Request> req;
    req.reset(treq);


    psm.release();          // Done with ProtoSocketMessage; release it

    shared_ptr<ReplyChannel> rc = make_shared<IPCSocketReplyChannel> (conn_proto_socket);
    
    rh_->Handle(req, rc);

    return 1;

  }






      



  //  Destructor, closes socket contained within socket_fd_
  IpcSocketRequestListener::~IpcSocketRequestListener() {

  }

}  // end namespace
