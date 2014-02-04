#include "request_listener/request_handler.h"

#include <glog/logging.h>

#include "request_listener/request_listener_exception.h"

namespace datto_linux_client {

RequestHandler::RequestHandler(std::shared_ptr<BackupManager> backup_manager)
    : backup_manager_(backup_manager) {}

void RequestHandler::Handle(const Request &request,
                            std::shared_ptr<ReplyChannel> reply_channel) {
  LOG(INFO) << "Handling request of type " << request.type();
  Reply reply;
  try {
    if (request.type() == Request::START_BACKUP) {
      std::string backup_uuid = backup_manager_->StartBackup(request.start_backup_request());
      reply.set_type(Reply::START_BACKUP);
      reply.mutable_start_backup_reply()->set_job_uuid(backup_uuid);
    } else if (request.type() == Request::STOP_BACKUP) {
      backup_manager_->StopBackup(request.stop_backup_request());
      reply.set_type(Reply::STOP_BACKUP);
    } else {
      // TODO
      throw RequestListenerException("Not implemented");
    }
  } catch (const std::exception &e) {
    reply.set_type(Reply::ERROR);
    reply.mutable_error_reply()->set_short_error(e.what());
  }
  reply_channel->SendReply(reply);
}

} //  datto_linux_client
