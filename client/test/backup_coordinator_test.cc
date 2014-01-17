#include "backup/backup_coordinator.h"

#include <thread>

#include <gtest/gtest.h>

namespace {

using ::datto_linux_client::BackupCoordinator;

TEST(BackupCoordinatorTest, Constructor) {
  BackupCoordinator bc(0);
}

TEST(BackupCoordinatorTest, BasicSignalTest) {
  BackupCoordinator bc(1);

  // Brings count to 0
  bc.SignalFinished();

  // Should fail to bring count to 1
  EXPECT_FALSE(bc.SignalMoreWorkToDo());
}

TEST(BackupCoordinatorTest, BasicCancelTest) {
  BackupCoordinator bc(1);

  EXPECT_FALSE(bc.IsCancelled());
  bc.Cancel();
  EXPECT_TRUE(bc.IsCancelled());
}

TEST(BackupCoordinatorTest, BasicFatalTest) {
  BackupCoordinator bc(1);

  EXPECT_FALSE(bc.IsCancelled());

  auto error = bc.GetFatalError();
  EXPECT_EQ(nullptr, error);


  std::exception_ptr e;
  try {
    throw std::runtime_error("Dummy");
  } catch (...) {
    e = std::current_exception();
  }

  bc.SetFatalError(e);
  EXPECT_TRUE(bc.IsCancelled());

  error = bc.GetFatalError();
  EXPECT_EQ(e, error);
}

TEST(BackupCoordinatorTest, SignalTest) {
  BackupCoordinator bc(3);

  // 2
  bc.SignalFinished();
  // 1
  bc.SignalFinished();
  // 2
  EXPECT_TRUE(bc.SignalMoreWorkToDo());
  // 1
  bc.SignalFinished();

  EXPECT_FALSE(bc.WaitUntilFinished(100));
  // 0
  bc.SignalFinished();
  // 0
  EXPECT_FALSE(bc.SignalMoreWorkToDo());
  EXPECT_TRUE(bc.WaitUntilFinished(100));
}



} // namespace
