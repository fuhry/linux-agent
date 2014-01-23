#include "backup/backup_coordinator.h"
#include "backup_status_tracker/backup_error.h"

#include <thread>

#include <gtest/gtest.h>

namespace {

using ::datto_linux_client::BackupCoordinator;
using ::datto_linux_client::BackupError;

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

  auto errors = bc.GetFatalErrors();
  EXPECT_EQ(0U, errors.size());

  BackupError e("dummy");
  bc.AddFatalError(e);
  EXPECT_TRUE(bc.IsCancelled());

  errors = bc.GetFatalErrors();
  ASSERT_EQ(1, errors.size());
  EXPECT_EQ(e, errors[0]);
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
