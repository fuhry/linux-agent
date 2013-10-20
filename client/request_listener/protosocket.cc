#include <protosocket.h>
#include <string>
#include <iostream>

#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

namespace datto_linux_client {

  using std::string;
  using std::shared_ptr;
  using std::cerr;
  using std::cout;
  using std::endl;

bool ProtoSocket::OpenUNIXListener(string &unix_path)  {

    unix_path_ = unix_path;
    const char *up = unix_path_.c_str();       //  For notational convenience
    struct sockaddr_un addr;
    int fd;

    // Grab a new socket
    fd = socket(AF_UNIX, SOCK_STREAM, 0); 
    if (fd < 0) {
      string err = "(UNIX) Error on socket() call: ";
      err = err + string(strerror(errno));
      throw ProtoSocketException(err);
    }

    // Remove old file if it exists; throw exception on error
    if (unlink(up) < 0 && errno != ENOENT) {
      string err = "(UNIX) Error on unlink for ";
      err = err + string(up) + string(": ");
      err = err + string(strerror(errno));
      throw ProtoSocketException(err);
    }

    memset(&addr, 0, sizeof(addr));    //  Set up sockaddr struct for bind/listen calls
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, up, sizeof(addr.sun_path) - 1);

    //  do bind(), throw exception if it fails
    if ( bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0) {
      string err = "(UNIX) Error on bind() call: ";
      err = err + string(strerror(errno));
      throw ProtoSocketException(err);
    }

    if ( listen(fd, 5) < 0) {
      string err = "(UNIX) Error on listen() call: ";
      err = err + string(strerror(errno));
      throw ProtoSocketException(err);
    }

    fd_ = fd;
    socket_type_ = UNIX_LISTENER;
    listen_port_ = 0;
    
    return true;

  }

bool ProtoSocket::OpenTCPListener(int port)  {

    struct sockaddr_in addr;
    int fd;

    listen_port_ = port;

    // Grab a new socket

    fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd  < 0) {
      string err = "(IP) Error on socket() call: ";
      err = err + string(strerror(errno));
      throw ProtoSocketException(err);
    }

    int on = 1;     // Used for turning on SO_REUSEADDR
    if ( setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, 
                    (const char *) &on, sizeof(on)) < 0 ) {
      string err = "(IP) Error on setsockopt() call: ";
      err = err + string(strerror(errno));
      throw ProtoSocketException(err);
    }


    memset(&addr, 0, sizeof(addr));    //  Set up sockaddr struct for bind/listen calls
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(listen_port_);

    //  do bind(), throw exception if it fails
    if ( bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) {
      string err = "(IP) Error on bind() call: ";
      err = err + string(strerror(errno));
      throw ProtoSocketException(err);
    }

    if ( listen(fd, 5) < 0) {
      string err = "(IP) Error on listen() call: ";
      err = err + string(strerror(errno));
      throw ProtoSocketException(err);
    }

    fd_ = fd;
    socket_type_ = TCP_LISTENER;
    unix_path_ = "";

    return true;

  }

  bool ProtoSocket::OpenUNIXConnection(string &unix_path) {

    struct sockaddr_un addr;
    int fd;


    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0 ) {
      string err = "(UNIX) Error on socket() call: ";
      err = err + string(strerror(errno));
      throw ProtoSocketException(err);
    }


    memset(&addr, 0, sizeof(addr));     // Clear the structure

    addr.sun_family = AF_UNIX;
    // strncpy( addr.sun_path, unix_path.c_str(), sizeof(addr.sun_path)-1 );
      strcpy( addr.sun_path, unix_path.c_str());
    int len = strlen(addr.sun_path) + sizeof(addr.sun_family);

    if ( connect(fd, (struct sockaddr *) &addr, len) < 0) {
      string err = "(UNIX) Error on connect() call: ";
      err = err + string(strerror(errno));
      throw ProtoSocketException(err);
    }

    fd_ = fd;
    socket_type_ = CONNECTION;
    listen_port_ = 0;

    return true;

  }

  bool ProtoSocket::OpenTCPConnection(string &remote_server, uint16_t remote_port) {

    // First get network-style address for the remote server

    struct addrinfo hints, *ai_res;
    char strport[30];
    int fd;

    sprintf(strport, "%hu", remote_port);

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = 0;

    int gai_ret = getaddrinfo( remote_server.c_str(), strport, &hints, &ai_res ); 
    if ( gai_ret < 0 ) {
      string err = "(TCP) error on getaddrinfo(): ";
      string gai_err = string(gai_strerror(gai_ret));
      err = err + gai_err; 
      throw ProtoSocketException(err);
    }

    // Note:  going to use just the first address returned by getaddrinfo for now

    fd = socket(ai_res->ai_family, ai_res->ai_socktype, ai_res->ai_protocol);
    if (fd < 0) {
      string err = "(TCP) socket() call: ";
      err = err + string(strerror(errno));
      throw ProtoSocketException(err);
    }

    if (connect(fd, ai_res->ai_addr, ai_res->ai_addrlen) < 0) {
      string err = "(TCP) connect() call: ";
      err = err + string(strerror(errno));
      throw ProtoSocketException(err);
    }

    fd_ = fd;
    socket_type_ = CONNECTION;
    unix_path_ = "";
    listen_port_ = 0;

    return true;

  }

  bool ProtoSocket::CanRead(int timeout) {

    struct pollfd pfd;

    if (fd_ <= 0) {
      string err = "Error: call to CanRead() on an unopened instance";
      throw ProtoSocketException(err);
    }

    memset(&pfd, 0, sizeof(pfd));

    pfd.fd = fd_;
    pfd.events =  POLLIN;

    int poll_ret = poll(&pfd, 1, timeout);   // returns number of ready fd's

    return (poll_ret == 0) ? false : true;

  }

  bool ProtoSocket::CanWrite(int timeout) {

    struct pollfd pfd;

    if (fd_ <= 0) {
      string err = "Error: call to CanWrite() on an unopened instance";
      throw ProtoSocketException(err);
    }

    memset(&pfd, 0, sizeof(pfd));

    pfd.fd = fd_;
    pfd.events =  POLLOUT;

    int poll_ret = poll(&pfd, 1, timeout);   // returns number of ready fd's

    return (poll_ret == 0) ? false : true;

  }

  shared_ptr<ProtoSocket> ProtoSocket::NewConnection() {

    if (socket_type_ != UNIX_LISTENER && socket_type_ != TCP_LISTENER) {
      string err = "Error: call to NewConnection() on a non-listener instance";
      throw ProtoSocketException(err);
    }

    if (fd_ <= 0) {
      string err = "Error: call to NewConnection() on an unopened instance";
      throw ProtoSocketException(err);
    }

    int newfd = accept(fd_, NULL, NULL);
    if (newfd < 0) {
      string err = "Error on call to accept(): ";
      err = err + string(strerror(errno));
      throw ProtoSocketException(err);
    }

    shared_ptr<ProtoSocket> tps( new ProtoSocket(newfd) );

    return tps;

  }

  bool ProtoSocket::SendMessage(ProtoSocketMessage &psm) {
    
    return SendMessage(psm.size, psm.buffer.get());

  }

  bool ProtoSocket::SendMessage(ProtoSocketMessage *psm) {
    
    return SendMessage(psm->size, psm->buffer.get());

  }

  bool ProtoSocket::SendMessage(uint32_t size, shared_ptr<unsigned char> buffer) {
    return SendMessage(size, buffer.get());
  }


  bool ProtoSocket::SendMessage(uint32_t size, unsigned char *  buffer) {

    int msg_size = size;

    int bytes_sent;
    uint32_t net_size = htonl(msg_size);

    bytes_sent = send(fd_, &net_size, sizeof(net_size), MSG_NOSIGNAL);

    if (bytes_sent < sizeof(net_size)) {
      string err = "Error: short send while sending message length";
      throw ProtoSocketException(err);
    }

    bytes_sent = send(fd_, buffer, msg_size, MSG_NOSIGNAL);

    if (bytes_sent < msg_size) {
      string err = "Error: short send while sending message";
      throw ProtoSocketException(err);
    }
    
    return true;

  }

  shared_ptr<ProtoSocketMessage> ProtoSocket::GetMessage() {

    uint32_t msg_len_net, msg_len_int;
    ssize_t  bytes_read;
    shared_ptr<unsigned char> buffer;

    shared_ptr<ProtoSocketMessage> psm(new ProtoSocketMessage);
    psm->size = 0;

    if (fd_ < 0) {
      return psm;      //  If not open, return zero-length PSM struct
    }

    bytes_read = recv(fd_, &msg_len_net, sizeof(msg_len_net), MSG_WAITALL);

    if (bytes_read < sizeof(msg_len_net)) {
      string err = "short read while reading message length ";
      throw ProtoSocketException(err);
    }

    msg_len_int = ntohl(msg_len_net);

    unsigned char *tbuf = (unsigned char *)malloc(msg_len_int);

    bytes_read = recv(fd_, tbuf, msg_len_int, MSG_WAITALL);
    buffer.reset(tbuf);

    if (bytes_read < sizeof(msg_len_net)) {
      if (errno) {
        string err = "short read while reading message ";
        throw ProtoSocketException(err);
      } else {
        return psm;
      }
    }

    psm->size = msg_len_int;
    psm->buffer = buffer;

    return psm;

  }

  bool ProtoSocket::CloseSocket() {

    if (fd_ <= 0) {
      close(fd_);
      fd_ = -1;
      return true;
    } else {
      return false;
    }

  }
    
  ProtoSocket::~ProtoSocket () {
    CloseSocket();
  }

  }  // end namespace ProtoSocket

