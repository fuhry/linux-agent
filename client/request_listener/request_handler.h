#ifndef DATTO_CLIENT_REQUEST_LISTENER_REQUEST_HANDLER_H_
#define DATTO_CLIENT_REQUEST_LISTENER_REQUEST_HANDLER_H_

#include <memory>

#include "backup/backup_manager.h"

#include "request_listener/reply_channel.h"
#include "request.pb.h"

namespace datto_linux_client {
class RequestHandler {
 public:
  RequestHandler(std::shared_ptr<BackupManager> backup_manager);
  virtual void Handle(const Request &request,
                      std::shared_ptr<ReplyChannel> reply_channel);
  virtual ~RequestHandler() {}

  RequestHandler(const RequestHandler&) = delete;
  RequestHandler& operator=(const RequestHandler&) = delete;
 protected:
  // For unit testing
  RequestHandler() {};

 private:
  std::shared_ptr<BackupManager> backup_manager_;
};
}

#endif //  DATTO_CLIENT_REQUEST_LISTENER_REQUEST_HANDLER_H_
