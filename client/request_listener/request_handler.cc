#include "request_listener/request_handler.h"
#include <glog/logging.h>

namespace datto_linux_client {

RequestHandler::RequestHandler(std::shared_ptr<BackupManager> backup_manager)
    : backup_manager_(backup_manager) {}

void RequestHandler::Handle(const Request &request,
                            std::shared_ptr<ReplyChannel> reply_channel) {
  LOG(INFO) << "Handling request of type " << request.type();
  Reply reply;
  if (request.type() == Request::START_BACKUP) {
    reply = backup_manager_->StartBackup(request.start_backup_request());
  } else if (request.type() == Request::STOP_BACKUP) {
    reply = backup_manager_->StopBackup(request.stop_backup_request());
  } else if (request.type() == Request::BACKUP_STATUS) {
    reply = backup_manager_->BackupStatus(request.backup_status_request());
  } else {
    // TODO
    throw std::runtime_error("Not implemented");
  }
  reply_channel->SendReply(reply);
}

} //  datto_linux_client
