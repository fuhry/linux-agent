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
  ~BackupCoordinator() {}

  // This should be called by DeviceSynchronizers when they are finished
  // synchronizing
  virtual void SignalFinished();

  // This is called to "undo" a prior SignalFinished call.
  //
  // Returns true if permitted to keep working, false if the backup
  // process is complete.
  virtual bool SignalMoreWorkToDo();

  virtual void SetFatalError(const std::exception_ptr exception);
  // null shared ptr means no fatal error
  // TODO exception pointer type exists
  virtual std::exception_ptr GetFatalError() const;

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
 protected:
  // For unit testing
  BackupCoordinator() {}

 private:
  unsigned int count_;
  mutable std::mutex mutex_;
  std::condition_variable cond_variable_;
  bool cancelled_;

  std::exception_ptr fatal_error_;
};

} // datto_linux_client

#endif  //  DATTO_CLIENT_BACKUP_BACKUP_COORDINATOR_H_
