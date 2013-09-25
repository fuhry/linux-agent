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

    uint32_t msg_len_int, msg_len_net;      // Message lengths
    ssize_t bytes_sent;
    char * buffer;

    msg_len_int = tm1->ByteSize();          // Get size
    msg_len_net = htonl(msg_len_int);       // Convert to network 32 bit int
    buffer = (char *)malloc(msg_len_int);

    tm1->SerializeToArray(buffer, msg_len_int);

    // DEBUG

    cout << "From serialized message:" << endl;
    cout << " txtmsg:   " << tm1->txtmsg() << endl;
    cout << " stamp:    " << tm1->stamp() << endl << endl;
    cout << "Calculated message size: " << msg_len_int << endl;


    bytes_sent = send(connection, &msg_len_net, sizeof(msg_len_net), MSG_NOSIGNAL);

    if ( bytes_sent < sizeof(msg_len_net) ) {
      perror("send() of length: ");
      exit(1);
    }

    bytes_sent = send(connection, buffer, msg_len_int, MSG_NOSIGNAL);
    free(buffer);

    if ( bytes_sent < sizeof(msg_len_net) ) {
      perror("send() of length: ");
      exit(1);
    }



    if (! strcmp(inbuf, "QUIT")) {
      break;
    }
    if (! strcmp(inbuf, "SHUT")) {
      break;
    }
  }


  close(connection);

}






















