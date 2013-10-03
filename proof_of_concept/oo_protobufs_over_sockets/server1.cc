#include <protobuf_classes/base_signature.pb.h>
#include <protobuf_classes/request.pb.h>
#include <protobuf_classes/start_backup_request.pb.h>
#include <protobuf_classes/stop_backup_request.pb.h>

#include <protosocket.h>
#include <threadstatus.h>

#include <iostream>

#include <string>
#include <map>
#include <memory>

#include <cstdlib>

#include <thread>
#include <mutex>
#include <chrono>

#include <signal.h>

using namespace std;

using std::string;
using std::unique_ptr;
using std::cout;
using std::cerr;
using std::endl;

using std::mutex;
using std::thread;

using ProtoSocket::ProtoSocketMessage;
using ProtoSocket::ProtoSocketException;

using datto_linux_client::Request;
using datto_linux_client::StartBackupRequest;;
using datto_linux_client::StopBackupRequest;;

using ThreadStatus::ThreadStatus;

//  Thread control variables
map<std::string, ThreadStatus::ThreadStatus *> threads;   
mutex thread_mutex;
std::chrono::milliseconds sleeptime(2000);

mutex cout_mutex;

int fake_backup(ThreadStatus::ThreadStatus * ts) {

  uint64_t counter = 0L;

  // Lock the mutex, grab our device name

  thread_mutex.lock();
  string device_name = ts->device_name();
  thread_mutex.unlock();

  cout_mutex.lock();
  cout << "Starting \"backup\" of " << device_name << endl;
  cout_mutex.unlock();

  for (;;) {    // fake a backup
    counter++;

    cout_mutex.lock();
    cout << "  for " << device_name << " counter is " << counter << endl;
    cout_mutex.unlock();

    std::this_thread::sleep_for(sleeptime);   // Sleep for a bit..  would actually do work here

    thread_mutex.lock();

    bool shutdown;
    if (ts->change_state()) {
      shutdown = ts->IsStopped();
    }

    thread_mutex.unlock();

    if (shutdown) {   //  Did we get a stop request?
      thread_mutex.lock();              // Lock the mutex
      threads.erase(device_name);       // Remove my entry from the map
      thread_mutex.unlock();
      cout_mutex.lock();
      cout << "*** Ending \"backup\" for device " << device_name << " ***" << endl;
      cout_mutex.unlock();
      break;
    }

  }

}

unique_ptr<ProtoSocket::ProtoSocket> ps_listen;

void sigint_handler(int signal) {

  cerr << "sigint_handler entered" << endl;
  ps_listen->CloseSocket();
  exit(0);

}

int main(int argc, char **argv) {

  const int LISTEN_WAIT_TIME = 1000 * 10;     //  Number of milliseconds to wait on listener
  const int CONN_WAIT_TIME = 1000 * 2;       //  Number of milliseconds to wait on connection

  if (argc < 3) {
    cerr << "Error: must provide arguments of \"TCP <portnum>\" or " <<
            "\"UNIX <pathname>\" to identify socket type and location" << endl;
    exit(1);
  } 

  struct sigaction act;
  memset(&act, 0, sizeof(struct sigaction));
  act.sa_handler = &sigint_handler;
  sigaction (SIGINT, &act, NULL);

  ProtoSocket::ProtoSocket *t = new ProtoSocket::ProtoSocket;
  ps_listen.reset(t);

  //
  //  Set up a listener
  //

  if (string(argv[1]) == "TCP") {

    int port = std::atoi(argv[2]);

    if (port < 1) {
      cerr << "Error: invalid port number given as second argument" << endl;
      exit(1);
    }

    try {
      ps_listen->OpenTCPListener(port);
    }
    catch (ProtoSocketException &e) {
      cerr << "ProtoSocketException thrown during OpenTCPListener: " <<
              e.what() << endl;
      exit(2);
    }
    catch (...) {
      cerr << "unspecified error thrown during OpenTCPListener: " << endl;
      exit(2);
    }

  } else {   // Assume UNIX socket if not TCP

    try {
      string path(argv[2]);
      ps_listen->OpenUNIXListener(path);
    }
    catch (ProtoSocketException &e) {
      cerr << "ProtoSocketException thrown during OpenUNIXListener: " <<
              e.what() << endl;
      exit(2);
    }
    catch (...) {
      cerr << "unspecified error thrown during OpenUNIXListener: " << endl;
      exit(2);
    }


  }

  for (;;) {      //  Handle connections as they come in 
    
    if (ps_listen->CanRead(LISTEN_WAIT_TIME)) {  //  returns true if a conn request is waiting

      unique_ptr<ProtoSocket::ProtoSocket> ps_conn;  // Grab a new ProtoSocket for the connection (does accept())
      try {
        ps_conn = ps_listen->NewConnection();
      }
      catch (ProtoSocketException &e) {
        cerr << "ProtoSocketException thrown during NewConnection: " <<
                e.what() << endl;
        exit(2);
      }
      catch (...) {
        cerr << "unspecified error thrown during NewConnection: " << endl;
        exit(2);
      }

      datto_linux_client::Request request_msg;   //  Google Protbuff object for the message

      for(;;) {

        unique_ptr<ProtoSocketMessage> inbuf;    //  Gets the length and content of the message

        try {
          inbuf = ps_conn->GetMessage();   // Just blocking and waiting for now
        }
        catch (ProtoSocketException &e) {
          cerr << "ProtoSocketException thrown during GetMessage: " <<
          e.what() << endl;
        }
        catch (...) {
          cerr << "unspecified error thrown during GetMessage: " << endl;
          exit(2);
        }

        if (inbuf->size== 0) {
          cerr << "Connection broken" << endl;
          break;
        }

        unsigned char *buff = (inbuf->buffer).get();   // Grab a copy of the buffer pointer
        request_msg.ParseFromArray(buff, inbuf->size);

        cout_mutex.lock();
        cout << "Received message.  Request type is ";  // * HERE WE HAVE A MESSAGE *
        cout_mutex.unlock();

        Request::RequestType rt = request_msg.type();

        if (rt == Request::START_BACKUP) {


          cout_mutex.lock();
          cout << "START_BACKUP" << endl;;
          cout_mutex.unlock();
          if (request_msg.has_start_backup_request()) {

            StartBackupRequest sbr(request_msg.start_backup_request());
            string device_name = sbr.block_path();

            cout_mutex.lock();
            cout << "  Backing up device " <<
                    device_name << endl;
            cout_mutex.unlock();

            //
            //  Here is where we handle the start-backup thread
            //

            ThreadStatus::ThreadStatus * ts = new ThreadStatus::ThreadStatus(device_name);
            ts->SetChangeState(false);
            ts->SetRunning();

            cout_mutex.lock();
            cout << "  starting new thread to back up " << device_name << endl;
            cout_mutex.unlock();

            thread_mutex.lock();
            threads[device_name] = ts;
            thread_mutex.unlock();

            thread newthread(fake_backup, ts);

            cout_mutex.lock();
            cout << "  new thread has id " << newthread.get_id() << endl;
            cout_mutex.unlock();

            newthread.detach();

          } else {
            cout << "  StartBackupRequest message was missing" << endl;
          }

        } else if (rt == Request::STOP_BACKUP) {

          cout_mutex.lock();
          cout << "STOP_BACKUP" << endl;;
          cout_mutex.unlock();

          if (request_msg.has_stop_backup_request()) {

            StopBackupRequest sbr(request_msg.stop_backup_request());

            string device_name = sbr.block_path();

            cout_mutex.lock();
            cout << "  Stopping back-up for device " <<
                    device_name << endl;
            cout_mutex.unlock();

            //
            //  Here is where we handle the stop-backup request
            //

            ThreadStatus::ThreadStatus * ts;

            if (threads.find(device_name) == threads.end()) {

              cout_mutex.lock();
              cout << "Error: attempt to stop non-existent backup for " << device_name << endl;
              cout_mutex.unlock();

            } else {

              thread_mutex.lock();
              ts = threads[device_name];
              ts->setStopped();
              ts->SetChangeState(true);
              thread_mutex.unlock();

              cout_mutex.lock();
              cout << "  signalled backup for " << device_name << "to stop" << endl;
              cout_mutex.unlock();
            }

          } else {
            cout << "  StopBackupRequest message was missing" << endl;
          }

        } else {
          cout << "*UNDEFINED*" << endl;
        }

      }

    } else {   // CanRead() timed out if we reach this point
      cerr << "Timed out waiting for connection request" << endl;
    }
  }

}

