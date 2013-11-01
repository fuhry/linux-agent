#ifndef DATTO_CLIENT_DATTOD_SIGNAL_HANDLER_H_
#define DATTO_CLIENT_DATTOD_SIGNAL_HANDLER_H_

#include <functional>
#include <vector>

#include <signal.h>

namespace datto_linux_client {

class SignalHandler {
 public:
  explicit SignalHandler(const std::vector<int> signals_to_handle);

  // Block the signals in the signal_set. If any are caught between the call to
  // BlockSignals and the call to WaitForSignal, the signal will be handled
  // immediately in WaitForSignal
  void BlockSignals();

  // Returns the signal caught
  int WaitForSignal(std::function<void(int)> to_run);

  ~SignalHandler();
 private:
  const std::vector<int> signals_to_handle_;
};

} // datto_linux_client

#endif //  DATTO_CLIENT_DATTOD_SIGNAL_HANDLER_H_
