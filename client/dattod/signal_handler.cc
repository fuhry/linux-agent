#include "dattod/signal_handler.h"
#include "dattod/dattod_exception.h"

#include <glog/logging.h>

namespace {

static volatile sig_atomic_t signal_caught = 0;

void sig_handler(int signal) {
  signal_caught = (sig_atomic_t)signal; 
}

struct sigaction get_default_sig_action() {
  struct sigaction sig_action;
  sig_action.sa_handler = SIG_DFL;
  sigemptyset(&sig_action.sa_mask);
  sig_action.sa_flags = 0;

  return sig_action;
}

}

namespace datto_linux_client {

SignalHandler::SignalHandler(std::vector<int> signals_to_handle)
    : signals_to_handle_(signals_to_handle) { }

void SignalHandler::BlockSignals() {
  struct sigaction sig_action = get_default_sig_action();
  sig_action.sa_handler = sig_handler;

  for (int signal : signals_to_handle_) {
    if (sigaction(signal, &sig_action, NULL)) {
      PLOG(ERROR) << "Unable to set signal " << signal;
      throw DattodException("Unable to set signal");
    }
  }
}

// Because the signal handler and the value it sets are static, if for some
// reason there are multiple instances of SignalHandler, it is first come
// first serve.
int SignalHandler::WaitForSignal(std::function<void(int)> to_run) {
  BlockSignals();

  // pause will return when any signal handler is called
  pause();

  // clear the static value
  int local_signal_caught = signal_caught;
  signal_caught = 0;

  // This would be an assert if we weren't so dependant on destructors for
  // cleanup
  if (local_signal_caught == 0) {
    throw DattodException("Unexpected! Signal was still 0 after handler");
  }

  to_run(local_signal_caught);
  return local_signal_caught;
}

// Reset all the signals to their defaults
SignalHandler::~SignalHandler() {
  struct sigaction default_action = get_default_sig_action();

  for (int signal : signals_to_handle_) {
    if (sigaction(signal, &default_action, NULL)) {
      PLOG(ERROR) << "Unable to set signal " << signal;
      // don't throw as we are in a destructor
    }
  }
}


} // datto_linux_client
