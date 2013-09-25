#include <t1.pb.h>
#include <fstream>
#include <iostream>
#include <string>

#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define SOCKET_FN "/tmp/testsock"

using namespace std;

int main(int argc, char **argv) {

  struct sockaddr_un addr;        // address structure for UNIX sockets
  int listener, connection;       // Socket FDs
  
  listener = socket(AF_UNIX, SOCK_STREAM, 0);     //  UNIX socket
  if (listener < 0) {
    perror("socket(): ");
    exit(1);
  }

  if (unlink(SOCKET_FN) < 0 && errno != ENOENT) {     // Get rid of file if it exists
    perror("unlink(): ");
    exit(1);
  }

  memset(&addr, 0, sizeof(addr));

  addr.sun_family = AF_UNIX;

  //  copy filename to addr struct, set rest of sun_path to nulls
  strncpy(addr.sun_path, SOCKET_FN, sizeof(addr.sun_path) - 1);

  if (bind(listener, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) < 0) {
    perror("bind(): ");
    exit(1);
  }

  if (listen(listener, 5) < 0) {
    perror("listen(): ");
    exit(1);
  }

  for (;;) {
    connection = accept(listener, NULL, NULL);
    if (connection < 0) {
      perror("accept(): ");
      exit(1);
    }

    for (;;) {

      TestMessage * tm1 = new TestMessage;      // Message contained in proto buffer
      unsigned char * buffer;                   // Input buffer
      uint32_t msg_len_net, msg_len_int;        // Length of message; received from client
      ssize_t bytes_in;                         // return value of recv() function

      bytes_in = recv(connection, &msg_len_net,
                      (size_t)sizeof(msg_len_net),
                      MSG_WAITALL);

      if ( bytes_in < sizeof(msg_len_net) ) {
        if (errno) {
          perror("recv() for length:");
          exit(1);
        } else {
          cout << "> client bailed!" << endl;
          break;
        }
      }

      msg_len_int = ntohl(msg_len_net);
      cout << "> received length: " << msg_len_int << endl;
      buffer = (unsigned char *)malloc(msg_len_int);   // Allocate buffer

      bytes_in = recv(connection, buffer, msg_len_int, MSG_WAITALL);  // Get whole message

      if ( bytes_in < msg_len_int ) {
        if (errno) {
          perror("recv() of message: ");
          exit(1);
        } else {
          cout << "> client bailed!" << endl;
          break;
        }
      }

      tm1->ParseFromArray(buffer, msg_len_int);
      free(buffer);

      cout << "Message received: " << endl;
      cout << " txtmsg:   " << tm1->txtmsg() << endl;
      cout << " stamp:    " << tm1->stamp() << endl << endl;

      if (tm1->txtmsg() == string("QUIT")) {
        cout << "** QUIT received, closing connection" << endl;
        close(connection);
        break;
      } else if (tm1->txtmsg() == string("SHUT")) {
        close(connection);
        cout << "** SHUT received, shutting down" << endl;
        exit(0);
        break;
      }
    }
  }



}






















