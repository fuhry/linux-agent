#include <protobuf_classes/base_signature.pb.h>
#include <protobuf_classes/request.pb.h>
#include <protobuf_classes/start_backup_request.pb.h>
#include <protobuf_classes/stop_backup_request.pb.h>

#include <protosocket.h>

#include <fstream>
#include <iostream>
#include <string>
#include <memory>

#include <cstdlib>

using namespace std;

using std::string;
using std::unique_ptr;
using std::cout;
using std::cerr;
using std::endl;

using ProtoSocket::ProtoSocketMessage;
using ProtoSocket::ProtoSocketException;

using datto_linux_client::Request;
using datto_linux_client::BaseSignature;
using datto_linux_client::StartBackupRequest;
using datto_linux_client::StopBackupRequest;

int main(int argc, char **argv) {

  if (argc < 3) {
    cerr << "Error: must provide arguments of \"TCP <server> <portnum>\" or " <<
            "\"UNIX <pathname>\" to identify socket type and location" << endl;
    exit(1);
  } 

  unique_ptr<ProtoSocket::ProtoSocket> ps_connect;
  ProtoSocket::ProtoSocket *t = new ProtoSocket::ProtoSocket;
  ps_connect.reset(t);

  if (string(argv[1]) == "TCP") {

    if (argc < 4) {
      cerr << "Error: must provide server and port number for TCP connections" << endl;
      exit(1);
    }

    int port = std::atoi(argv[3]);

    if (port < 1) {
      cerr << "Error: invalid port number given as third argument" << endl;
      exit(1);
    }

    string server(argv[2]);

    try {
      ps_connect->OpenTCPConnection(server, port);
    }
    catch (ProtoSocketException &e) {
      cerr << "ProtoSocketException thrown during OpenTCPConnection: " <<
              e.what() << endl;
      exit(2);
    }

  } else {   // Assume UNIX socket if not TCP

    try {
      string path(argv[2]);
      ps_connect->OpenUNIXConnection(path);
    }
    catch (ProtoSocketException &e) {
      cerr << "ProtoSocketException thrown during OpenUNIXConnection: " <<
              e.what() << endl;
      exit(2);
    }

  }

  for (;;) {      //  Send messages to server until stopped

    char cmd_in[100];

    cout << endl << "Enter START, STOP, or QUIT:" << endl;

    cin.getline(cmd_in, 99);

    string cmd(cmd_in);

    if (cmd.substr(0, 5) == string("START")) {

      cmd_in[0] = 0;
      cout << "Enter a device name:" << endl;
      cin.getline(cmd_in, 99);
      cmd = cmd_in;
      if (cmd.length() == 0) {
        cout << "Error: must enter a device name" << endl;
        break;
      } 

      Request req_msg;
      StartBackupRequest *sbr_msg = req_msg.mutable_start_backup_request();;

      sbr_msg->set_block_path(cmd.c_str());
      sbr_msg->set_type(StartBackupRequest::FULL_BACKUP);

      req_msg.set_type(Request::START_BACKUP);

      ProtoSocketMessage psm;
      uint32_t t_len = req_msg.ByteSize();
      unsigned char * t_msg = (unsigned char *) malloc(t_len);

      req_msg.SerializeToArray(t_msg, t_len);
      unique_ptr<unsigned char> t_msg_ptr(t_msg);

      psm.size = t_len;
      psm.buffer = std::move(t_msg_ptr);

      try {
        ps_connect->SendMessage(psm);
      }
      catch (ProtoSocketException &e) {
        cerr << "ProtoSocketException thrown by SendMessage: " << e.what() << endl;
        exit(2);
      }

    } else if (cmd.substr(0, 4) == string("STOP")) {

      cmd_in[0] = 0;
      cout << "Enter a device name:" << endl;
      cin.getline(cmd_in, 99);
      cmd = cmd_in;
      if (cmd.length() == 0) {
        cout << "Error: must enter a device name" << endl;
        break;
      } 

      Request req_msg;
      StopBackupRequest *sbr_msg = req_msg.mutable_stop_backup_request();;

      sbr_msg->set_block_path(cmd.c_str());

      req_msg.set_type(Request::STOP_BACKUP);

      ProtoSocketMessage psm;
      uint32_t t_len = req_msg.ByteSize();
      unsigned char * t_msg = (unsigned char *) malloc(t_len);

      req_msg.SerializeToArray(t_msg, t_len);
      unique_ptr<unsigned char> t_msg_ptr(t_msg);

      psm.size = t_len;
      psm.buffer = std::move(t_msg_ptr);

      try {
        ps_connect->SendMessage(psm);
      }
      catch (ProtoSocketException &e) {
        cerr << "ProtoSocketException thrown by SendMessage: " << e.what() << endl;
        exit(2);
      }

    } else if (cmd.substr(0, 4) == string("QUIT")) {
      cout << "Received quit request... exiting";
      exit(0);
    } else {
      cerr << "Invalid input \"" << cmd << "\" given" << endl;
    }

  }


}

