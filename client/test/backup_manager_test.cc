#include "backup/backup_manager.h"

#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

using ::datto_linux_client::Backup;
using ::datto_linux_client::BackupBuilder;
using ::datto_linux_client::BackupEventHandler;
using ::datto_linux_client::BackupManager;
using ::datto_linux_client::BackupStatusTracker;
using ::datto_linux_client::BlockDevice;
using ::datto_linux_client::UnsyncedSectorManager;
using ::datto_linux_client::UnsyncedSectorStore;

using ::datto_linux_client::DevicePair;
using ::datto_linux_client::StartBackupRequest;
using ::datto_linux_client::StopBackupRequest;
using ::datto_linux_client::BackupStatusRequest;
using ::datto_linux_client::BackupStatusReply;

using ::testing::Eq;
using ::testing::Expectation;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::Test;
using ::testing::Throw;
using ::testing::_;

class MockBackupBuilder : public BackupBuilder {
  MOCK_METHOD1(CreateBackup,
               std::shared_ptr<Backup>(std::vector<DevicePair>));
};

class MockUnsyncedSectorManager : public UnsyncedSectorManager {
  MOCK_METHOD1(StartTracer, void(const BlockDevice &));
  MOCK_CONST_METHOD1(IsTracing, bool(const BlockDevice &));
  MOCK_METHOD1(GetStore,
               std::shared_ptr<UnsyncedSectorStore>(const BlockDevice &));
};

class MockBackupStatusTracker : public BackupStatusTracker {
  MOCK_METHOD1(GetReply,
               std::shared_ptr<BackupStatusReply>(const std::string &job_id));
  MOCK_METHOD1(CreateEventHandler,
               std::shared_ptr<BackupEventHandler>(const std::string &job_id));
};

class BackupManagerTest : public ::testing::Test {
 protected:
  BackupManagerTest() {
  }

  std::shared_ptr<BackupBuilder> backup_builder;
  std::shared_ptr<UnsyncedSectorManager> sector_manager;
  std::shared_ptr<BackupStatusTracker> status_tracker;
};
} // anonymous namespace

TEST_F(BackupManagerTest, Constructor) {
  BackupManager bm(backup_builder, sector_manager, status_tracker);
}
