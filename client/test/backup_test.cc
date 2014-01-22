#include "backup/backup.h"

#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "backup/backup_coordinator.h"
#include "backup_status_tracker/backup_event_handler.h"
#include "backup_status_tracker/sync_count_handler.h"
#include "device_synchronizer/device_synchronizer_exception.h"
#include "device_synchronizer/device_synchronizer_interface.h"
#include "unsynced_sector_manager/sector_interval.h"

namespace {

using ::datto_linux_client::Backup;
using ::datto_linux_client::BackupCoordinator;
using ::datto_linux_client::BackupEventHandler;
using ::datto_linux_client::BlockDevice;
using ::datto_linux_client::DeviceSynchronizerException;
using ::datto_linux_client::DeviceSynchronizerInterface;
using ::datto_linux_client::MountableBlockDevice;
using ::datto_linux_client::SectorSet;
using ::datto_linux_client::SyncCountHandler;
using ::datto_linux_client::UnsyncedSectorManager;

using ::testing::Eq;
using ::testing::Expectation;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::Test;
using ::testing::Throw;
using ::testing::_;

class MockDeviceSynchronizer : public DeviceSynchronizerInterface {
 public:
  MOCK_METHOD2(DoSync, void(std::shared_ptr<BackupCoordinator> coordinator,
                            std::shared_ptr<SyncCountHandler> count_handler));

  MOCK_CONST_METHOD0(source_device,
                     std::shared_ptr<const MountableBlockDevice>());

  MOCK_CONST_METHOD0(source_unsynced_manager,
                     std::shared_ptr<const UnsyncedSectorManager>());

  MOCK_CONST_METHOD0(destination_device,
                     std::shared_ptr<const BlockDevice>());
};

class MockBackupCoordinator : public BackupCoordinator {
 public:
  MOCK_METHOD0(SignalFinished, void());
  MOCK_METHOD0(SignalMoreWorkToDo, bool());
  MOCK_METHOD1(AddFatalError, void(const std::exception_ptr error));
  MOCK_CONST_METHOD0(GetFatalErrors, std::vector<std::exception_ptr>());
  MOCK_METHOD0(Cancel, void());
  MOCK_CONST_METHOD0(IsCancelled, bool());
  MOCK_METHOD1(WaitUntilFinished, bool(int timeout_millis));
};

class MockBackupEventHandler : public BackupEventHandler {
 public:
  MOCK_METHOD0(BackupCopying, void());
  MOCK_METHOD0(BackupFinished, void());
  MOCK_METHOD0(BackupCancelled, void());
  MOCK_METHOD1(BackupFailed, void(const std::string &failure_message));
  MOCK_METHOD1(CreateSyncCountHandler,
               std::shared_ptr<SyncCountHandler>(
                   const std::string &block_device_name));
};

class MockSyncCountHandler : public SyncCountHandler {
 public:
  MOCK_METHOD1(UpdateSyncedCount, void(uint64_t num_synced));
  MOCK_METHOD1(UpdateUnsyncedCount, void(uint64_t num_unsynced));
};

class MockMountableBlockDevice : public MountableBlockDevice {
 public:
  MOCK_CONST_METHOD0(path, std::string());
  MOCK_METHOD0(GetInUseSectors, std::shared_ptr<const SectorSet>());
};

class BackupTest : public ::testing::Test {
 protected:
  BackupTest() {
    source_device = std::make_shared<MockMountableBlockDevice>();
    event_handler = std::make_shared<MockBackupEventHandler>();
    coordinator = std::make_shared<MockBackupCoordinator>();
    sync_count_handler = std::make_shared<MockSyncCountHandler>();
  }

  std::shared_ptr<MockMountableBlockDevice> source_device;
  std::shared_ptr<MockSyncCountHandler> sync_count_handler;
  std::shared_ptr<MockBackupCoordinator> coordinator;
  std::shared_ptr<MockBackupEventHandler> event_handler;
};
} // anonymous namespace

TEST_F(BackupTest, Constructor) {
  Backup b(std::vector<std::shared_ptr<DeviceSynchronizerInterface>>());
}

TEST_F(BackupTest, NothingToDo) {
  auto work = std::vector<std::shared_ptr<DeviceSynchronizerInterface>>();
  Backup b(work);
  b.InsertBackupCoordinator(coordinator);
  b.DoBackup(event_handler);
  EXPECT_CALL(*coordinator, WaitUntilFinished(_))
      .WillOnce(Return(true));
}

TEST_F(BackupTest, SingleSyncToDo) {
  auto device_sync = std::make_shared<MockDeviceSynchronizer>();

  EXPECT_CALL(*device_sync, DoSync(Eq(coordinator), Eq(sync_count_handler)))
      .Times(1);
  EXPECT_CALL(*device_sync, source_device())
      .WillRepeatedly(Return(source_device));
  EXPECT_CALL(*coordinator, WaitUntilFinished(_))
      .WillOnce(Return(true));
  EXPECT_CALL(*event_handler, CreateSyncCountHandler(_))
      .WillOnce(Return(sync_count_handler));

  std::vector<std::shared_ptr<DeviceSynchronizerInterface>> work =
      {device_sync};

  Backup b(work);
  b.InsertBackupCoordinator(coordinator);
  b.DoBackup(event_handler);
}

TEST_F(BackupTest, HandlesException) {
  auto device_sync = std::make_shared<MockDeviceSynchronizer>();
  DeviceSynchronizerException to_throw("Test exception");
  auto to_throw_pointer = std::make_exception_ptr(to_throw);
  std::vector<std::exception_ptr> errors = {to_throw_pointer};

  Expectation add_error = EXPECT_CALL(*coordinator,
                                      AddFatalError(to_throw_pointer));
  EXPECT_CALL(*coordinator, GetFatalErrors())
      .After(add_error)
      .WillOnce(Return(errors));
  EXPECT_CALL(*coordinator, WaitUntilFinished(_))
      .WillOnce(Return(true));
  EXPECT_CALL(*event_handler, CreateSyncCountHandler(_))
      .WillOnce(Return(sync_count_handler));

  EXPECT_CALL(*event_handler, BackupCopying());
  EXPECT_CALL(*event_handler, BackupFailed(_));
  EXPECT_CALL(*event_handler, BackupFinished())
      .Times(0);

  EXPECT_CALL(*device_sync, DoSync(Eq(coordinator), Eq(sync_count_handler)))
      .Times(1)
      .WillOnce(Throw(to_throw));

  std::vector<std::shared_ptr<DeviceSynchronizerInterface>> work =
      {device_sync};

  Backup b(work);
  b.DoBackup(event_handler);
}

TEST_F(BackupTest, HandlesMany) {
  Sequence seq1;
  Sequence seq2;
  auto event_handler1 = std::make_shared<MockBackupEventHandler>();
  auto event_handler2 = std::make_shared<MockBackupEventHandler>();
  EXPECT_CALL(*event_handler1, CreateSyncCountHandler(_))
      .WillOnce(Return(sync_count_handler));
  EXPECT_CALL(*event_handler1, BackupCopying())
      .InSequence(seq1);
  EXPECT_CALL(*event_handler1, BackupFinished())
      .InSequence(seq1);
  EXPECT_CALL(*event_handler2, CreateSyncCountHandler(_))
      .WillOnce(Return(sync_count_handler));
  EXPECT_CALL(*event_handler2, BackupCopying())
      .InSequence(seq2);
  EXPECT_CALL(*event_handler2, BackupFinished())
      .InSequence(seq2);

  auto device_sync1 = std::make_shared<MockDeviceSynchronizer>();
  auto device_sync2 = std::make_shared<MockDeviceSynchronizer>();

  EXPECT_CALL(*coordinator, WaitUntilFinished(_))
      .WillOnce(Return(true));

  EXPECT_CALL(*device_sync1, DoSync(Eq(coordinator), Eq(sync_count_handler)))
      .Times(1);
  EXPECT_CALL(*device_sync2, DoSync(Eq(coordinator), Eq(sync_count_handler)))
      .Times(1);

  std::vector<std::shared_ptr<DeviceSynchronizerInterface>> work =
      {device_sync1, device_sync2};

  Backup b(work);
  b.DoBackup(event_handler);
}
