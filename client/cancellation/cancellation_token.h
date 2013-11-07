#ifndef DATTO_CLIENT_CANCELLATION_CANCELLATION_TOKEN_H_
#define DATTO_CLIENT_CANCELLATION_CANCELLATION_TOKEN_H_

#include <atomic>

namespace datto_linux_client {

class CancellationToken {
 public:
  CancellationToken();

  void Cancel();
  bool ShouldCancel();

  ~CancellationToken() {}

  CancellationToken(const CancellationToken&) = delete;
  CancellationToken& operator=(const CancellationToken&) = delete;
 private:
  std::atomic<bool> should_cancel_;
};

} // datto_linux_client

#endif  // DATTO_CLIENT_CANCELLATION_CANCELLATION_TOKEN_H_
