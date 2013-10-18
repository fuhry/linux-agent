#include <iostream>
#include <exception>
#include <test_request_handler.h>
#include "request.pb.h"
#include "reply.pb.h"

namespace datto_linux_client {

  using namespace std;

  TestRequestHandler::TestRequestHandler() {
  }

  TestRequestHandler::~TestRequestHandler() {
  }

  void TestRequestHandler::Handle(shared_ptr<Request> request,
                                  shared_ptr<ReplyChannel> reply_channel) {


    if (! request->has_type() ) {     //  No request type set
      string err = "Error: Request buffer received with no RequestType";
      throw TestRequestHandlerException(err);  
    }

    switch (request->type()) {

      case Request::START_BACKUP :
        handle_start(request, reply_channel);
        break;

      case Request::STOP_BACKUP :
        handle_stop(request, reply_channel);
        break;

      case Request::STATUS_BACKUP :
        handle_status(request, reply_channel);
        break;

      case Request::PAUSE_BACKUP :
        handle_pause(request, reply_channel);
        break;

      default :
        string err = "Error: undefined RequestType in Request buffer";
        throw TestRequestHandlerException(err);
    }

    cout << "After the call to handle_xxxx in TestRequestHandler, about to return" << endl;

    return;

  }

  void TestRequestHandler::handle_start(shared_ptr<Request> request,
                                        shared_ptr<ReplyChannel> reply_channel) {

    cout << "START request received; entered handle_start" << endl;

    if ( ! request->has_start_backup_request() ) {
      string err = "Error: Start request does not contain a StartBackupRequest";
      throw TestRequestHandlerException(err);
    }

    StartBackupRequest sbr = request->start_backup_request();

    if ( ! sbr.has_type() ) {
      string err = "Error: Start request does not contain a RequestType";
      throw TestRequestHandlerException(err);
    }

    cout << "Backup type is ";

    switch (sbr.type()) {
      case StartBackupRequest::FULL_BACKUP :
        cout << "FULL";
        break;
      case StartBackupRequest::INCREMENTAL_BACKUP :
        cout << "INCREMENTAL";
        break;
      case StartBackupRequest::DIFF_BACKUP :
        cout << "DIFF";
        break;
      default :
        string err = "Start Backup request had an invalid RequestType";
        throw TestRequestHandlerException(err);
    }

    cout << endl;

    if ( ! sbr.has_block_path() ) {
      string err = "Start request missing block_path";
      throw TestRequestHandlerException(err);
    }

    cout << "Backup requested for device " <<
            sbr.block_path() << endl;

    //  Send a reply

    shared_ptr<Reply> reply(new Reply);

    reply->set_type(Reply::START_BACKUP);
    StartBackupReply * subrep = reply->mutable_start_backup_reply();
    
    subrep->set_started(true);
    subrep->set_rc(0);
    subrep->set_block_path(sbr.block_path());
    subrep->set_errmsg(string("Success"));

    reply_channel->SendReply(reply);

  }

  void TestRequestHandler::handle_stop(shared_ptr<Request> request,
                                       shared_ptr<ReplyChannel> reply_channel) {

    cout << "STOP request received; entered handle_stop" << endl;

    if ( ! request->has_stop_backup_request() ) {
      string err = "Error: Stop request does not contain a StopBackupRequest";
      throw TestRequestHandlerException(err);
    }

    StopBackupRequest sbr = request->stop_backup_request();


    if ( ! sbr.has_block_path() ) {
      string err = "Stop request missing block_path";
      throw TestRequestHandlerException(err);
    }

    cout << "Stop backup requested for device " <<
            sbr.block_path() << endl;

    //  Send a reply

    shared_ptr<Reply> reply(new Reply);

    reply->set_type(Reply::STOP_BACKUP);
    StopBackupReply * subrep = reply->mutable_stop_backup_reply();
    
    subrep->set_stopped(true);
    subrep->set_rc(0);
    subrep->set_block_path(sbr.block_path());
    subrep->set_errmsg(string("Success"));

    reply_channel->SendReply(reply);

  }


  void TestRequestHandler::handle_pause(shared_ptr<Request> request,
                                       shared_ptr<ReplyChannel> reply_channel) {

    cout << "PAUSE request received; entered handle_pause" << endl;

    if ( ! request->has_pause_backup_request() ) {
      string err = "Error: Pause request does not contain a PauseBackupRequest";
      throw TestRequestHandlerException(err);
    }

    PauseBackupRequest pbr = request->pause_backup_request();


    if ( ! pbr.has_block_path() ) {
      string err = "Pause request missing block_path";
      throw TestRequestHandlerException(err);
    }

    cout << "Pause backup requested for device " <<
            pbr.block_path() << endl;

    //  Send a reply

    shared_ptr<Reply> reply(new Reply);

    reply->set_type(Reply::PAUSE_BACKUP);
    PauseBackupReply * subrep = reply->mutable_pause_backup_reply();
    
    subrep->set_paused(true);
    subrep->set_rc(0);
    subrep->set_block_path(pbr.block_path());
    subrep->set_errmsg(string("Success"));

    reply_channel->SendReply(reply);
  }


  void TestRequestHandler::handle_status(shared_ptr<Request> request,
                                       shared_ptr<ReplyChannel> reply_channel) {

    cout << "STATUS request received; entered handle_status" << endl;

    if ( ! request->has_status_backup_request() ) {
      string err = "Error: Status request does not contain a StatusBackupRequest";
      throw TestRequestHandlerException(err);
    }

    StatusBackupRequest sbr = request->status_backup_request();


    if ( ! sbr.has_block_path() ) {
      string err = "Status request missing block_path";
      throw TestRequestHandlerException(err);
    }

    cout << "Status backup requested for device " <<
            sbr.block_path() << endl;

    //  Send a reply

    shared_ptr<Reply> reply(new Reply);

    reply->set_type(Reply::STATUS_BACKUP);
    StatusBackupReply * subrep = reply->mutable_status_backup_reply();
    
    subrep->set_backup_state(StatusBackupReply::RUNNING);
    subrep->set_block_path(sbr.block_path());
    subrep->add_return_codes(0);
    subrep->add_messages(string("success"));

    subrep->set_completed_blocks(0);
    subrep->set_total_blocks(1000000L);

    reply_channel->SendReply(reply);
  }

}  // end of namespace



  



    




