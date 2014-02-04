#ifndef DATTO_CLIENT_BACKUP_STATUS_TRACKER_BACKUP_ERROR_H_
#define DATTO_CLIENT_BACKUP_STATUS_TRACKER_BACKUP_ERROR_H_

#include <string>

namespace datto_linux_client {

class BackupError {
 public:
  explicit BackupError(const std::string &error_text_a)
      : error_text_(error_text_a) {}

  BackupError(const std::string &error_text_a,
              const std::string &error_source_a)
      : error_text_(error_text_a),
        error_source_(error_source_a) {}

  BackupError(const std::string &error_text_a,
              const std::string &error_source_a,
              const std::string &error_trace_a)
      : error_text_(error_text_a),
        error_source_(error_source_a),
        error_trace_(error_trace_a) {}

  std::string error_text() const {
    return error_text_;
  }

  std::string error_source() const {
    return error_source_;
  }

  std::string error_trace() const {
    return error_trace_;
  }

  bool operator==(const BackupError &other) const {
    return (error_text_ == other.error_text()
            && error_source_ == other.error_source()
            && error_trace_ == other.error_trace());
  }

 private:
  std::string error_text_;
  std::string error_source_;
  std::string error_trace_;
};

} // datto_linux_client

#endif  // DATTO_CLIENT_BACKUP_STATUS_TRACKER_BACKUP_ERROR_H_
