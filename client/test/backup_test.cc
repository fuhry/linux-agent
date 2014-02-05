#include "backup/backup.h"

#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "backup/backup_coordinator.h"
#include "backup_status_tracker/backup_error.h"
#include "backup_status_tracker/backup_event_handler.h"
#include "backup_status_tracker/sync_count_handler.h"
#include "device_synchronizer/device_synchronizer_exception.h"
#include "device_synchronizer/device_synchronizer_interface.h"
#include "unsynced_sector_manager/sector_interval.h"

namespace {

using ::datto_linux_client::Backup;
using ::datto_linux_client::BackupCoordinator;
using ::datto_linux_client::BackupError;
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
  MOCK_CONST_METHOD0(sector_manager,
                     std::shared_ptr<const UnsyncedSectorManager>());
  MOCK_CONST_METHOD0(destination_device,
                     std::shared_ptr<const BlockDevice>());
};

class MockBackupCoordinator : public BackupCoordinator {
 public:
  MOCK_METHOD1(AddFatalError, void(const BackupError &error));
  MOCK_CONST_METHOD0(GetFatalErrors, std::vector<BackupError>());
  MOCK_METHOD1(WaitUntilFinished, bool(int timeout_millis));
};

class MockBackupEventHandler : public BackupEventHandler {
 public:
  MOCK_METHOD0(BackupInProgress, void());
  MOCK_METHOD0(BackupSucceeded, void());
  MOCK_METHOD0(BackupCancelled, void());
  MOCK_METHOD1(BackupFailed, void(const std::string &failure_message));
  MOCK_METHOD1(CreateSyncCountHandler,
               std::shared_ptr<SyncCountHandler>(
                   const MountableBlockDevice &block_device));
};

class MockSyncCountHandler : public SyncCountHandler {
 public:
  MOCK_METHOD1(UpdateSyncedCount, void(uint64_t num_synced));
  MOCK_METHOD1(UpdateUnsyncedCount, void(uint64_t num_unsynced));
};

class MockMountableBlockDevice : public MountableBlockDevice {
 public:
  MOCK_CONST_METHOD0(uuid, std::string());
  MOCK_CONST_METHOD0(path, std::string());
  MOCK_METHOD0(GetInUseSectors, std::shared_ptr<const SectorSet>());
};

class BackupTest : public ::testing::Test {
 protected:
  BackupTest() {
    source_device = std::make_shared<MockMountableBlockDevice>();
    sync_count_handler = std::make_shared<MockSyncCountHandler>();
    event_handler = std::make_shared<MockBackupEventHandler>();
    coordinator = std::make_shared<MockBackupCoordinator>();
  }

  std::shared_ptr<MockMountableBlockDevice> source_device;
  std::shared_ptr<MockSyncCountHandler> sync_count_handler;
  std::shared_ptr<MockBackupCoordinator> coordinator;
  std::shared_ptr<MockBackupEventHandler> event_handler;
};
} // anonymous namespace

TEST_F(BackupTest, Constructor) {
  std::vector<std::shared_ptr<DeviceSynchronizerInterface>> syncs_to_do;
  Backup b(syncs_to_do, coordinator);
}

TEST_F(BackupTest, NothingToDo) {
  std::vector<BackupError> no_errors;
  EXPECT_CALL(*coordinator, GetFatalErrors())
      .WillOnce(Return(no_errors));

  EXPECT_CALL(*coordinator, WaitUntilFinished(_))
      .WillOnce(Return(true));

  EXPECT_CALL(*event_handler, BackupInProgress());
  EXPECT_CALL(*event_handler, BackupSucceeded());

  auto work = std::vector<std::shared_ptr<DeviceSynchronizerInterface>>();
  Backup b(work, coordinator);
  b.DoBackup(event_handler);
}

TEST_F(BackupTest, SingleSyncToDo) {
  auto device_sync = std::make_shared<MockDeviceSynchronizer>();
  std::vector<BackupError> no_errors;
  EXPECT_CALL(*coordinator, GetFatalErrors())
      .WillOnce(Return(no_errors));

  EXPECT_CALL(*event_handler, CreateSyncCountHandler(_))
      .WillOnce(Return(sync_count_handler));
  EXPECT_CALL(*device_sync, DoSync(Eq(coordinator), Eq(sync_count_handler)));
  EXPECT_CALL(*device_sync, source_device())
      .WillRepeatedly(Return(source_device));
  EXPECT_CALL(*coordinator, WaitUntilFinished(_))
      .WillOnce(Return(true));
  EXPECT_CALL(*event_handler, BackupInProgress());
  EXPECT_CALL(*event_handler, BackupSucceeded());

  std::vector<std::shared_ptr<DeviceSynchronizerInterface>> work =
      {device_sync};

  Backup b(work, coordinator);
  b.DoBackup(event_handler);
}

TEST_F(BackupTest, HandlesException) {
  auto device_sync = std::make_shared<MockDeviceSynchronizer>();
  DeviceSynchronizerException to_throw("Test exception");
  BackupError expected_error(std::string(to_throw.what()), "dummy-uuid");
  std::vector<BackupError> errors = {expected_error};

  Expectation add_error = EXPECT_CALL(*coordinator,
                                      AddFatalError(Eq(expected_error)));
  EXPECT_CALL(*coordinator, GetFatalErrors())
      .After(add_error)
      .WillOnce(Return(errors));
  EXPECT_CALL(*coordinator, WaitUntilFinished(_))
      .WillOnce(Return(true));
  EXPECT_CALL(*source_device, uuid())
      .WillRepeatedly(Return("dummy-uuid"));
  EXPECT_CALL(*device_sync, source_device())
      .WillRepeatedly(Return(source_device));
  EXPECT_CALL(*event_handler, CreateSyncCountHandler(_))
      .WillOnce(Return(sync_count_handler));

  EXPECT_CALL(*event_handler, BackupInProgress());
  EXPECT_CALL(*event_handler, BackupFailed(_));
  EXPECT_CALL(*event_handler, BackupSucceeded())
      .Times(0);

  EXPECT_CALL(*device_sync, DoSync(Eq(coordinator), Eq(sync_count_handler)))
      .WillOnce(Throw(to_throw));

  std::vector<std::shared_ptr<DeviceSynchronizerInterface>> work =
      {device_sync};

  Backup b(work, coordinator);
  b.DoBackup(event_handler);
}

TEST_F(BackupTest, HandlesMany) {
  EXPECT_CALL(*event_handler, CreateSyncCountHandler(_))
      .Times(2)
      .WillRepeatedly(Return(sync_count_handler));
  EXPECT_CALL(*event_handler, BackupInProgress());
  EXPECT_CALL(*event_handler, BackupSucceeded());

  std::vector<BackupError> no_errors;
  EXPECT_CALL(*coordinator, GetFatalErrors())
      .WillOnce(Return(no_errors));

  auto device_sync1 = std::make_shared<MockDeviceSynchronizer>();
  auto device_sync2 = std::make_shared<MockDeviceSynchronizer>();

  EXPECT_CALL(*device_sync1, source_device())
      .WillRepeatedly(Return(source_device));
  EXPECT_CALL(*device_sync2, source_device())
      .WillRepeatedly(Return(source_device));

  EXPECT_CALL(*coordinator, WaitUntilFinished(_))
      .WillOnce(Return(true));

  EXPECT_CALL(*device_sync1, DoSync(Eq(coordinator), Eq(sync_count_handler)));
  EXPECT_CALL(*device_sync2, DoSync(Eq(coordinator), Eq(sync_count_handler)));

  std::vector<std::shared_ptr<DeviceSynchronizerInterface>> work =
      {device_sync1, device_sync2};

  Backup b(work, coordinator);
  b.DoBackup(event_handler);
}
