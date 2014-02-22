#include <memory>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <glog/logging.h>

#include "backup/backup_builder.h"
#include "backup/backup_manager.h"
#include "backup_status_tracker/backup_status_tracker.h"
#include "block_device/block_device_factory.h"
#include "dattod/flock.h"
#include "dattod/signal_handler.h"
#include "request_listener/ipc_request_listener.h"
#include "request_listener/request_handler.h"

const char DATTO_VAR_DIR[] = "/var/lib/datto";
const char DATTO_SOCKET[] = "/var/lib/datto/dattod_ipc";
const char FLOCK_PATH[] = "/var/lib/datto/dattod.pid";

namespace {
using datto_linux_client::BackupBuilder;
using datto_linux_client::BackupManager;
using datto_linux_client::BackupStatusTracker;
using datto_linux_client::BlockDeviceFactory;
using datto_linux_client::Flock;
using datto_linux_client::IpcRequestListener;
using datto_linux_client::RequestHandler;
using datto_linux_client::SignalHandler;
using datto_linux_client::UnsyncedSectorManager;
}

int main(int argc, char *argv[]) {
  // Setup logging

  // Only log to a memory location until a LogSink is written that doesn't
  // write when the file system is frozen!
  FLAGS_log_dir = "/dev/shm/";
  google::InitGoogleLogging(argv[0]);

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

  auto block_device_factory = std::make_shared<BlockDeviceFactory>();
  auto sector_manager = std::make_shared<UnsyncedSectorManager>();
  auto backup_builder = std::make_shared<BackupBuilder>(block_device_factory,
                                                        sector_manager);
  auto status_tracker = std::make_shared<BackupStatusTracker>();

  // Create the backup manager
  auto backup_manager = std::make_shared<BackupManager>(backup_builder,
                                                        sector_manager,
                                                        status_tracker);
  // Create the request handler
  std::unique_ptr<RequestHandler> request_handler(
      new RequestHandler(backup_manager, status_tracker));
  // Create (and start) the request listener
  IpcRequestListener request_listener(DATTO_SOCKET,
                                      std::move(request_handler));

  // Listen for signals
  signal_handler.WaitForSignal([&](int) {} );

  return 0;
}
