#include "logging/queuing_log_sink.h"

#include <chrono>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace {
  using google::LogSeverity;
  using google::GetLogSeverityName;
} // namespace

namespace datto_linux_client {

QueuingLogSink::QueuingLogSink(std::string output_file_name)
    : google::LogSink(),
      to_write_queue_(),
      mutex_(),
      cond_variable_(),
      should_die_(false) {
  // group and other shouldn't touch the log file
  mode_t old_mask = umask(077);

  FILE *output_file = fopen(output_file_name.c_str(), "a");

  if (output_file == NULL) {
    // TODO real exception
    throw std::runtime_error("Unable to open logging file " +
                             std::string(strerror(errno)));
  }

  writer_thread_ = std::thread([=]() {
    while (!this->should_die_) {
      std::unique_lock<std::mutex> lock(this->mutex_);
      auto timeout = std::chrono::milliseconds(500);
      this->cond_variable_.wait_for(lock, timeout);

      while (!this->to_write_queue_.empty()) {
        std::string message = this->to_write_queue_.front();
        this->to_write_queue_.pop();
        // Make sure we are _not_ holding the lock before hitting the
        // disk in case the write call blocks due to a frozen filesystem.
        lock.unlock();
        fputs(message.c_str(), output_file);
        fputs("\n", output_file);
        lock.lock();
      }
    }
    fclose(output_file);
  });

  umask(old_mask);
}

void QueuingLogSink::send(LogSeverity severity, const char *full_filename,
                          const char *base_filename, int line,
                          const struct ::tm *tm_time,
                          const char *message, size_t message_len) {
  std::string log_string = LogSink::ToString(severity, base_filename, line,
                                             tm_time, message, message_len);
  {
    std::lock_guard<std::mutex> lock(mutex_);
    to_write_queue_.push(log_string);
  }
  cond_variable_.notify_one();
}

QueuingLogSink::~QueuingLogSink() {
  should_die_ = true;
  writer_thread_.join();
}

} // datto_linux_client
