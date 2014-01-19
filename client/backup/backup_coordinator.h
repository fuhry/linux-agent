#ifndef DATTO_CLIENT_BACKUP_BACKUP_COORDINATOR_H_
#define DATTO_CLIENT_BACKUP_BACKUP_COORDINATOR_H_

#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <vector>

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
  ~BackupCoordinator() {}

  // This should be called by DeviceSynchronizers when they are finished
  // synchronizing
  virtual void SignalFinished();

  // This is called to "undo" a prior SignalFinished call.
  //
  // Returns true if permitted to keep working, false if the backup
  // process is complete.
  virtual bool SignalMoreWorkToDo();

  virtual void AddFatalError(const std::exception_ptr exception);

  // Multiple Fatal Errors can occur, so track them all
  virtual std::vector<std::exception_ptr> GetFatalErrors() const;

  virtual void Cancel();
  // Returns true if explicitly cancelled with Cancel(), or if a 
  // fatal error was set
  virtual bool IsCancelled() const;

  // Waits for all synchronization to be done
  //
  // Only positive timeouts are permitted.
  //
  // return value of true means synchronziation is complete, false means it
  // timed out
  virtual bool WaitUntilFinished(int timeout_millis);

  BackupCoordinator(const BackupCoordinator &) = delete;
  BackupCoordinator& operator=(const BackupCoordinator &) = delete;
 protected:
  // For unit testing
  BackupCoordinator() {}

 private:
  unsigned int count_;
  mutable std::mutex mutex_;
  std::condition_variable cond_variable_;
  bool cancelled_;
  std::vector<std::exception_ptr> fatal_errors_;
};

} // datto_linux_client

#endif  //  DATTO_CLIENT_BACKUP_BACKUP_COORDINATOR_H_
