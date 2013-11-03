#include "request_listener/request_handler.h"
#include <glog/logging.h>

namespace datto_linux_client {

RequestHandler::RequestHandler(std::shared_ptr<BackupManager> backup_manager)
    : backup_manager_(backup_manager) {}

void RequestHandler::Handle(const Request &request,
                            std::shared_ptr<ReplyChannel> reply_channel) {
  LOG(INFO) << "Handling request of type " << request.type();
  if (request.type() == Request::START_BACKUP) {
    backup_manager_->StartFullBackup(request.start_backup_request(),
                                     reply_channel);
  } else {
    // TODO
    throw std::runtime_error("Not implemented");
  }
}
 
} //  datto_linux_client
