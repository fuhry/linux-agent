#include <t1.pb.h>
#include <fstream>
#include <iostream>
#include <string>

#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

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


    TestMessage * tm1 = new TestMessage;

    for (;;) {

      tm1->ParseFromFileDescriptor(connection);

      cout << "Message received: " << endl;
      cout << " txtmsg:   " << tm1->txtmsg() << endl;
      cout << " stamp:    " << tm1->stamp() << endl << endl;

      if (tm1->txtmsg() == string("QUIT")) {
        cout << "** QUIT received, leaving" << endl;
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






















