#ifndef DATTO_CLIENT_LOGGING_QUEUING_LOG_SINK_H_
#define DATTO_CLIENT_LOGGING_QUEUING_LOG_SINK_H_

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include <glog/logging.h>

namespace datto_linux_client {

// QueuingLogSink is designed to prevent the situation
// where we have frozen the hard drive and attempt to log
// at the same time (thus hanging the program)
//
// In order to prevent logging to a file, FLAGS_logtostderr
// ** must ** be set to true.
class QueuingLogSink : public google::LogSink {
 public:
  explicit QueuingLogSink(std::string output_file_name);
  
  virtual void send(google::LogSeverity severity, const char *full_filename,
                    const char *base_filename, int line,
                    const struct ::tm *tm_time,
                    const char *message, size_t message_len);

  ~QueuingLogSink();
 private:
  std::thread writer_thread_;

  std::queue<std::string> to_write_queue_;

  std::mutex mutex_;
  std::condition_variable cond_variable_;

  std::atomic<bool> should_die_;
};
} // datto_linux_client

#endif //  DATTO_CLIENT_LOGGING_QUEUING_LOG_SINK_H_
