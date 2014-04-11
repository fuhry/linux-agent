#include "backup/backup_manager.h"

#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "backup/backup_coordinator.h"
#include "start_backup_request.pb.h"

namespace {

using ::datto_linux_client::Backup;
using ::datto_linux_client::BackupBuilder;
using ::datto_linux_client::BackupCoordinator;
using ::datto_linux_client::BackupEventHandler;
using ::datto_linux_client::BackupManager;
using ::datto_linux_client::BackupStatusTracker;
using ::datto_linux_client::BlockDevice;
using ::datto_linux_client::UnsyncedSectorManager;
using ::datto_linux_client::UnsyncedSectorStore;

using ::datto_linux_client::BackupStatusReply;
using ::datto_linux_client::BackupStatusRequest;
using ::datto_linux_client::Vector;
using ::datto_linux_client::Reply;
using ::datto_linux_client::StartBackupReply;
using ::datto_linux_client::StartBackupRequest;
using ::datto_linux_client::StopBackupReply;
using ::datto_linux_client::StopBackupRequest;

using ::testing::DoAll;
using ::testing::Eq;
using ::testing::ElementsAre;
using ::testing::Expectation;
using ::testing::InvokeWithoutArgs;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::Sequence;
using ::testing::Test;
using ::testing::Throw;
using ::testing::_;

// Generated with "uuidgen" command
std::string DUMMY_FS_UUID0 = "6431d6c8-bbdf-4d46-b251-6ec32c95665a";
std::string DUMMY_FS_UUID1 = "e7948ed9-0367-4130-a330-2dad1d47affe";
std::string DUMMY_BACKUP_UUID0 = "eda148d1-3d6f-4b14-b557-3b5c8ffe9175";
std::string DUMMY_BACKUP_UUID1 = "5b8893ed-c64f-4ea6-b1bb-0ac27f846864";

class MockBackup : public Backup {
 public:
  MOCK_METHOD1(DoBackup, void(std::shared_ptr<BackupEventHandler>));
};

class MockBackupCoordinator : public BackupCoordinator {
 public:
};

class MockBackupBuilder : public BackupBuilder {
 public:
  MOCK_METHOD3(CreateBackup, std::shared_ptr<Backup>(
        const std::vector<Vector> &,
        const std::shared_ptr<BackupCoordinator> &,
        bool));
};

class MockUnsyncedSectorManager : public UnsyncedSectorManager {
 public:
  MOCK_METHOD1(StartTracer, void(const BlockDevice &));
  MOCK_CONST_METHOD1(IsTracing, bool(const BlockDevice &));
  MOCK_METHOD1(GetStore,
               std::shared_ptr<UnsyncedSectorStore>(const BlockDevice &));
};

class MockBackupStatusTracker : public BackupStatusTracker {
 public:
  MOCK_METHOD1(GetReply,
               std::shared_ptr<BackupStatusReply>(const std::string &job_id));
  MOCK_METHOD1(CreateEventHandler,
               std::shared_ptr<BackupEventHandler>(const std::string &job_id));
};

class BackupManagerTest : public ::testing::Test {
 protected:
  BackupManagerTest() {
    backup_builder = std::make_shared<MockBackupBuilder>();
    sector_manager = std::make_shared<MockUnsyncedSectorManager>();
    status_tracker = std::make_shared<MockBackupStatusTracker>();

    sleep_func = [&]() { sleep(1); };
  }

  std::function<void()> sleep_func;
  std::shared_ptr<BackupStatusTracker> status_tracker;
  std::shared_ptr<MockBackupBuilder> backup_builder;
  std::shared_ptr<UnsyncedSectorManager> sector_manager;
};

StartBackupRequest make_start_backup_request() {
  StartBackupRequest sr;
  sr.set_type(StartBackupRequest::FULL_BACKUP);

  Vector *dp1 = sr.add_vectors();
  Vector *dp2 = sr.add_vectors();

  dp1->set_block_device_uuid(DUMMY_FS_UUID0);
  dp1->set_destination_host("localhost");
  dp1->set_destination_port(12345);

  dp2->set_block_device_uuid(DUMMY_FS_UUID1);
  dp2->set_destination_host("localhost");
  dp2->set_destination_port(12345);

  return sr;
}

} // anonymous namespace

TEST_F(BackupManagerTest, Constructor) {
  BackupManager bm(backup_builder, sector_manager, status_tracker);
}

TEST_F(BackupManagerTest, ThrowsOnNoDevices) {
  BackupManager bm(backup_builder, sector_manager, status_tracker);

  StartBackupRequest sr;
  sr.set_type(StartBackupRequest::FULL_BACKUP);

  try {
    auto job_guid = bm.StartBackup(sr);
    FAIL() << "Should have failed with empty request";
  } catch (...) {
    // good
  }
}

TEST_F(BackupManagerTest, StartBackupRequest) {
  auto backup = std::make_shared<MockBackup>();

  EXPECT_CALL(*backup, DoBackup(_))
    .WillOnce(InvokeWithoutArgs(sleep_func));

  EXPECT_CALL(*backup_builder, CreateBackup(_, _, true))
    .Times(1)
    .WillOnce(Return(backup));

  auto start_request = make_start_backup_request();

  BackupManager bm(backup_builder, sector_manager, status_tracker);
  std::string uuid = bm.StartBackup(start_request);
}

TEST_F(BackupManagerTest, StartBackupRequestFailsOnRepeat) {
  auto backup = std::make_shared<MockBackup>();

  EXPECT_CALL(*backup, DoBackup(_))
    .WillOnce(InvokeWithoutArgs(sleep_func));

  EXPECT_CALL(*backup_builder, CreateBackup(_, _, true))
    .Times(1)
    .WillOnce(Return(backup));

  auto start_request = make_start_backup_request();

  BackupManager bm(backup_builder, sector_manager, status_tracker);
  bm.StartBackup(start_request);
  try {
    bm.StartBackup(start_request);
    FAIL() << "Should have thrown";
  } catch (...) {
    // good
  }
}

TEST_F(BackupManagerTest, StopThrowsIfNotStarted) {
  BackupManager bm(backup_builder, sector_manager, status_tracker);
  StopBackupRequest stop_request;
  stop_request.set_job_uuid(DUMMY_BACKUP_UUID0);
  try {
    bm.StopBackup(stop_request);
    FAIL() << "Should have thrown";
  } catch (...) {
    // good
  }
}

TEST_F(BackupManagerTest, StopBackupRequest) {
  std::shared_ptr<BackupCoordinator> coordinator;

  auto backup = std::make_shared<MockBackup>();

  EXPECT_CALL(*backup, DoBackup(_))
    .WillOnce(InvokeWithoutArgs(sleep_func));

  EXPECT_CALL(*backup_builder, CreateBackup(_, _, true))
    .WillOnce(DoAll(SaveArg<1>(&coordinator),
                    Return(backup)));

  auto start_request = make_start_backup_request();
  BackupManager bm(backup_builder, sector_manager, status_tracker);
  std::string backup_uuid = bm.StartBackup(start_request);

  EXPECT_FALSE(coordinator->IsCancelled());

  StopBackupRequest stop_request;
  stop_request.set_job_uuid(backup_uuid);
  bm.StopBackup(stop_request);

  // BackupManager should call Cancel on the coordinator
  EXPECT_TRUE(coordinator->IsCancelled());
}
