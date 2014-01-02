#include "dattod/flock.h"
#include "dattod/signal_handler.h"
#include "request_listener/ipc_request_listener.h"
#include "request_listener/request_handler.h"
#include "backup/backup_manager.h"

#include <glog/logging.h>

#include <memory>

#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>

const char DATTO_VAR_DIR[] = "/var/datto";
const char DATTO_SOCKET[] = "/var/datto/dattod_ipc";
const char FLOCK_PATH[] = "/var/datto/dattod.pid";

namespace {
using datto_linux_client::Flock;
using datto_linux_client::RequestHandler;
using datto_linux_client::SignalHandler;
using datto_linux_client::BackupManager;
using datto_linux_client::IpcRequestListener;
}

int main(int argc, char *argv[]) {
  // Setup logging

  // Only log to a memory location until a LogSink is written that doesn't
  // write when the file system is frozen!
  FLAGS_log_dir = "/dev/shm/";
  google::InitGoogleLogging(argv[0]);

  // TODO: Parse the config file

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

#ifdef NDEBUG
  // daemonize
  // (1, 0) means don't chdir but do release stdin/stdout/stderr
  if (daemon(1, 0)) {
    PLOG(ERROR) << "Unable to daemonize";
    return 1;
  }
#else
  FLAGS_stderrthreshold = 0;
#endif

  // Acquire lock (must happen after daemon() as daemon() changes the pid)
  std::unique_ptr<Flock> lock;
  try {
    lock = std::unique_ptr<Flock>(new Flock(FLOCK_PATH));
    lock->WritePid();
  } catch (const std::runtime_error &e) {
    LOG(ERROR) << e.what();
    return 1;
  }

  // Block typical death signals
  std::vector<int> signals_to_block { SIGTERM, SIGINT };
  SignalHandler signal_handler(signals_to_block);
  signal_handler.BlockSignals();

  // Create the backup manager
  std::shared_ptr<BackupManager> backup_manager(new BackupManager());
  // Create the request handler
  std::unique_ptr<RequestHandler> request_handler(
      new RequestHandler(backup_manager));
  // Create (and start) the request listener
  IpcRequestListener request_listener(DATTO_SOCKET,
                                      std::move(request_handler));

  // Listen for signals
  signal_handler.WaitForSignal([&](int) {} );

  // TODO: Stop everything that needs to be stopped
  return 0;
}
