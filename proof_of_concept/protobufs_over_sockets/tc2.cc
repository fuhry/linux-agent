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
  int connection;                 // Socket FD
  char inbuf[200];
  TestMessage * tm1 = new TestMessage;
  int stamp_counter = 1;

  if ( (connection = socket(AF_UNIX, SOCK_STREAM, 0) ) < 0) {
    perror("socket(): ");
    exit(1);
  }

  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, SOCKET_FN);
  int len = strlen(addr.sun_path) + sizeof(addr.sun_family);


  *inbuf = 0;

  if (connect(connection, (struct sockaddr *) &addr, len) < 0) {
    perror("connect(): ");
    exit(1);
  }


  for(;;) {

    cout << "Connected: type message (use \"QUIT\" to exit and shutdown server)" << endl;

    cin.getline(inbuf, 199);

    tm1->set_txtmsg(string(inbuf));
    tm1->set_stamp(stamp_counter++);

    tm1->SerializeToFileDescriptor(connection);


    if (! strcmp(inbuf, "QUIT")) {
      break;
    }
  }


  close(connection);

}






















