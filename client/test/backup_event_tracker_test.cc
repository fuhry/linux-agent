#include "backup_event_tracker/backup_event_tracker.h"
#include "backup_event_tracker/backup_event_handler.h"
#include "backup_status_reply.pb.h"

#include <gtest/gtest.h>

namespace {

using ::datto_linux_client::BackupEventTracker;
using ::datto_linux_client::BackupEventHandler;
using ::datto_linux_client::BackupStatusReply;

TEST(BackupEventTrackerTest, Constructor) {
  BackupEventTracker tracker;
}

TEST(BackupEventTrackerTest, NullPtrWhenNoReply) {
  BackupEventTracker tracker;
  auto result = tracker.GetReply("guid-dne");
  EXPECT_EQ(nullptr, result);
}

TEST(BackupEventTrackerTest, ReplyIsCreated) {
  BackupEventTracker tracker;
  auto handler = tracker.CreateEventHandler("test-guid");
  auto reply = tracker.GetReply("test-guid");
  EXPECT_NE(nullptr, reply);
}

TEST(BackupEventTrackerTest, ReplyIsUpdated) {
  BackupEventTracker tracker;
  auto handler = tracker.CreateEventHandler("test-guid");

  handler->BackupPreparing();
  auto reply = tracker.GetReply("test-guid");
  EXPECT_EQ(BackupStatusReply::PREPARING, reply->status());

  handler->BackupCopying();
  // Old reply should not have changed
  EXPECT_EQ(BackupStatusReply::PREPARING, reply->status());
  reply = tracker.GetReply("test-guid");
  // New reply should have changed
  EXPECT_EQ(BackupStatusReply::COPYING, reply->status());
}

TEST(BackupEventTrackerTest, SyncCountsWork) {
  BackupEventTracker tracker;
  auto handler = tracker.CreateEventHandler("test-guid");

  handler->UpdateSyncedCount(1234);
  auto reply = tracker.GetReply("test-guid");
  EXPECT_EQ(1234U, reply->bytes_transferred());

  handler->UpdateUnsyncedCount(4321);
  reply = tracker.GetReply("test-guid");
  // TODO: EXPECT_EQ(4321, reply->???());
}

TEST(BackupEventTrackerTest, FailureMessage) {
  BackupEventTracker tracker;
  auto handler = tracker.CreateEventHandler("test-guid");

  handler->BackupFailed("Test message");
  auto reply = tracker.GetReply("test-guid");
  EXPECT_EQ(BackupStatusReply::FAILED, reply->status());
  EXPECT_EQ("Test message", reply->failure_message());
}

TEST(BackupEventTrackerTest, MultipleHandlers) {
  BackupEventTracker tracker;
  auto handler1 = tracker.CreateEventHandler("test-guid1");
  auto handler2 = tracker.CreateEventHandler("test-guid2");

  handler1->BackupFailed("Test message");
  handler2->BackupSucceeded();

  auto reply1 = tracker.GetReply("test-guid1");
  auto reply2 = tracker.GetReply("test-guid2");

  EXPECT_EQ(BackupStatusReply::FAILED, reply1->status());
  EXPECT_EQ(BackupStatusReply::FINISHED, reply2->status());
}

} // namespace
