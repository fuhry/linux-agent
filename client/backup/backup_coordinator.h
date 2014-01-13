#ifndef DATTO_CLIENT_BACKUP_BACKUP_COORDINATOR_H_
#define DATTO_CLIENT_BACKUP_BACKUP_COORDINATOR_H_

#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>

namespace datto_linux_client {

// This class keeps count of the number of working threads.
// When the initial count (initialized with num_workers) is 0, it will not
// allow further increments. This way, once everything is synced we are done
// and shouldn't allow further work.
//
// There is also support for cancellation and error handling added to this.
// Their usage is straight forward from the method comments.
class BackupCoordinator {
 public:
  explicit BackupCoordinator(int num_workers);

  // This should be called by DeviceSynchronizers when they are finished
  // synchronizing
  void SignalFinished();

  // This is called to "undo" a prior SignalFinished call.
  //
  // Returns true if permitted to keep working, false if the backup
  // process is complete.
  bool SignalMoreWorkToDo();

  void SetFatalError(const std::exception &exception);
  // null shared ptr means no fatal error
  // TODO exception pointer type exists
  std::shared_ptr<std::exception> GetFatalError();

  void Cancel();
  // Returns true if explicitly cancelled with Cancel(), or if a 
  // fatal error was set
  bool IsCancelled();

  // Waits for all synchronization to be done
  //
  // Only positive timeouts are permitted.
  //
  // return value of true means synchronziation is complete, false means it
  // timed out
  bool WaitUntilFinished(int timeout_millis);

 private:
  unsigned int count_;
  std::mutex mutex_;
  std::condition_variable cond_variable_;
  bool cancelled_;

  std::shared_ptr<std::exception> fatal_error_;
};

} // datto_linux_client

#endif  //  DATTO_CLIENT_BACKUP_BACKUP_COORDINATOR_H_
