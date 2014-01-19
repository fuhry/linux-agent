#include "backup/backup.h"

#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "backup/backup_coordinator.h"
#include "backup_status_tracker/backup_event_handler.h"

namespace {

using ::datto_linux_client::BackupCoordinator;
using ::datto_linux_client::BackupEventHandler;
using ::datto_linux_client::DeviceSynchronizer;
using ::testing::Test;

class MockDeviceSynchronizer : public DeviceSynchronizerInterface {
 public:
  MockDeviceSynchronizer() {}
  MOCK_METHOD2(DoSync void(std::shared_ptr<BackupCoordinator>,
                           std::shared_ptr<BackupEventHandler>));
};

class MockBackupCoordinator : public BackupCoordinator {
 public:
  MockBackupCoordinator() {}

  MOCK_METHOD0(SignalFinished, void());
  MOCK_METHOD0(SignalMoreWorkToDo, bool());
  MOCK_METHOD1(AddFatalError, void(const std::exception_ptr));
  MOCK_CONST_METHOD0(GetFatalErrors, std::vector<std::exception_ptr>());
  MOCK_METHOD0(Cancel, void());
  MOCK_CONST_METHOD0(IsCancelled, bool());
  MOCK_METHOD1(WaitUntilFinished, bool(int));
};

class MockBackupEventHandler : public BackupEventHandler {
 public:
  MockBackupEventHandler() {}

  MOCK_METHOD0(BackupCopying, void());
  MOCK_METHOD0(BackupFinished, void());
  MOCK_METHOD0(BackupCancelled, void());
  MOCK_METHOD1(BackupFailed, void(const std::string &failure_message));
  MOCK_METHOD1(UpdateSyncedCount, void(uint64_t num_synced));
  MOCK_METHOD1(UpdateUnsyncedCount, void(uint64_t num_unsynced));
};

class BackupTest : public ::testing::Test {
 protected:
  BackupTest() {
    event_handler = std::make_shared<MockBackupEventHandler>();
  }

  std::shared_ptr<EventHandler> event_handler;
};
} // anoymous namespace

TEST_F(BackupTest, Constructor) {
  Backup b(std::vector<MockDeviceSynchronizer>());
}

TEST_F(BackupTest, NothingToDo) {
  Backup b(std::vector<MockDeviceSynchronizer>());
  b.DoBackup(event_handler);
}

TEST_F(BackupTest, SingleSyncToDo) {
  auto device_sync = std::make_shared<MockDeviceSynchronizer>();

  EXPECT_CALL(*device_sync, DoSync(event_handler)).Times(1);

  std::vector<std::shared_ptr<DeviceSynchronizer>> work = {device_sync};

  Backup b(work);
  b.DoBackup(event_handler);
}

// TODO: Errors
//       Many syncs
