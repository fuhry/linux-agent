#include "dattod/flock.h"
#include "request_listener/request_handler.h"

#include <glog/logging.h>

#include <memory>

#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>

const char DATTO_VAR_DIR[] = "/var/datto";
const char FLOCK_PATH[] = "/var/datto/dattod.pid";

namespace {
using datto_linux_client::Flock;
using datto_linux_client::RequestHandler;
}

int main(int argc, char *argv[]) {
  // Setup logging
  google::InitGoogleLogging(argv[0]);

  // Parse the config file

  // Make the DATTO_VAR_DIR directory
  // Save any error, we don't care about it unless chdir fails
  mkdir(DATTO_VAR_DIR, S_IRWXU | S_IRWXG);
  int mkdir_error = errno;

  // cd to that dir
  if (chdir(DATTO_VAR_DIR)) {
    PLOG(ERROR) << "Error during chdir to " << DATTO_VAR_DIR;
    LOG(ERROR) << "mkdir had error: " << strerror(mkdir_error);
    return 1;
  }

  // daemonize
  // (1, 0) means don't chdir but do release stdin/stdout/stderr
  if (daemon(1, 0)) {
    PLOG(ERROR) << "Unable to daemonize";
    return 1;
  }

  // Acquire lock (must happen after daemon() as daemon() changes the pid)
  try {
    Flock lock(FLOCK_PATH);
  } catch (const std::runtime_error &e) {
    LOG(ERROR) << e.what();
    return 1;
  }

  // Create the request handler
  RequestHandler handler();

  // Block typical death signals
  std::vector<int> signals_to_block { SIGTERM, SIGINT };
  SignalHandler signal_handler(signals_to_block);
  signal_handler.BlockSignals();

  // Start device tracer threads
  // Start the request listener thread

  // Listen for signals
  signal_handler.WaitForSignal();

  // Stop everything that needs to be stopped
  return 0;
}
