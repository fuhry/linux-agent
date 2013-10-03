#ifndef PROTO_SOCKET
#define PROTO_SOCKET


#include <string>

#include <stdexcept>
#include <memory>

//#include <sys/un.h>
//#include <sys/socket.h>
//#include <unistd.h>
//#include <errno.h>
//#include <arpa/inet.h>
//
namespace ProtoSocket {

  using std::string;
  using std::unique_ptr;

class ProtoSocketException : public std::runtime_error {
  public :
    explicit ProtoSocketException(std::string &what) : runtime_error(what){};
};

struct ProtoSocketMessage {
  uint32_t size;
  unique_ptr<unsigned char> buffer;
};


class ProtoSocket {

  private: 

    enum {
      UNDEFINED, 
      UNIX_LISTENER,
      TCP_LISTENER,
      CONNECTION
    } socket_type_;

    int fd_;
    string unix_path_;
    uint16_t listen_port_;  // a short, in other words 

  public:

    ProtoSocket(ProtoSocket const&)=delete;          //  Make non-copyable
    ProtoSocket& operator=(ProtoSocket const&) = delete;

    inline explicit ProtoSocket(int fd) :   // make with a connection socket
      socket_type_(CONNECTION), fd_(fd), unix_path_(""), listen_port_(-1) {};
    inline explicit ProtoSocket() :         //  Allocate only, must set type 
      socket_type_(UNDEFINED), fd_(-1), unix_path_(""), listen_port_(-1){};    

    bool OpenUNIXListener(string &unix_path);
    bool OpenUNIXConnection(string &unix_path);

    bool OpenTCPListener(int port);
    bool OpenTCPConnection(string &remote_server, uint16_t remote_port);
    inline bool SetConnection(int fd)  {
      fd_ = fd;
    }

    bool CanRead(int timeout = 0);        //  Anything to read?

    bool CanWrite(int timeout = 0);       //  Able to send without blocking?


    unique_ptr<ProtoSocketMessage> GetMessage();   //  Read a message.  CAN BLOCK!

    bool SendMessage(ProtoSocketMessage &psm);
    bool SendMessage(ProtoSocketMessage *psm);
    bool SendMessage(uint32_t size, unique_ptr<unsigned char>  buffer);

    unique_ptr<ProtoSocket> NewConnection();   //  accept(), etc.  CAN BLOCK!

    bool CloseSocket();
    ~ProtoSocket();

};

}   // namespace ProtoSocket

#endif //PROTO_SOCKET



    


  


