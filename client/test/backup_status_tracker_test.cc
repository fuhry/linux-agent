#include "backup_status_tracker/backup_status_tracker.h"
#include "backup_status_tracker/backup_event_handler.h"
#include "backup_status_reply.pb.h"

#include <gtest/gtest.h>

namespace {

using ::datto_linux_client::BackupStatusTracker;
using ::datto_linux_client::BackupEventHandler;
using ::datto_linux_client::BackupStatusReply;

TEST(BackupStatusTrackerTest, Constructor) {
  BackupStatusTracker tracker;
}

TEST(BackupStatusTrackerTest, NullPtrWhenNoReply) {
  BackupStatusTracker tracker;
  auto result = tracker.GetReply("uuid-dne");
  EXPECT_EQ(nullptr, result);
}

TEST(BackupStatusTrackerTest, ReplyIsCreated) {
  BackupStatusTracker tracker;
  auto handler = tracker.CreateEventHandler("test-uuid");
  auto reply = tracker.GetReply("test-uuid");
  EXPECT_NE(nullptr, reply);
}

TEST(BackupStatusTrackerTest, ReplyIsUpdated) {
  BackupStatusTracker tracker;
  auto handler = tracker.CreateEventHandler("test-uuid");

  handler->BackupCopying();
  auto reply = tracker.GetReply("test-uuid");
  EXPECT_EQ(BackupStatusReply::COPYING, reply->status());

  handler->BackupFinished();
  // Old reply should not have changed
  EXPECT_EQ(BackupStatusReply::COPYING, reply->status());
  reply = tracker.GetReply("test-uuid");
  // New reply should have changed
  EXPECT_EQ(BackupStatusReply::FINISHED, reply->status());
}

TEST(BackupStatusTrackerTest, SyncCountsWork) {
  BackupStatusTracker tracker;
  auto handler = tracker.CreateEventHandler("test-uuid");

  handler->UpdateSyncedCount(1234);
  auto reply = tracker.GetReply("test-uuid");
  EXPECT_EQ(1234U, reply->bytes_transferred());

  handler->UpdateUnsyncedCount(4321);
  reply = tracker.GetReply("test-uuid");
  // TODO: EXPECT_EQ(4321, reply->???());
}

TEST(BackupStatusTrackerTest, FailureMessage) {
  BackupStatusTracker tracker;
  auto handler = tracker.CreateEventHandler("test-uuid");

  handler->BackupFailed("Test message");
  auto reply = tracker.GetReply("test-uuid");
  EXPECT_EQ(BackupStatusReply::FAILED, reply->status());
  EXPECT_EQ("Test message", reply->failure_message());
}

TEST(BackupStatusTrackerTest, MultipleHandlers) {
  BackupStatusTracker tracker;
  auto handler1 = tracker.CreateEventHandler("test-uuid1");
  auto handler2 = tracker.CreateEventHandler("test-uuid2");

  handler1->BackupFailed("Test message");
  handler2->BackupFinished();

  auto reply1 = tracker.GetReply("test-uuid1");
  auto reply2 = tracker.GetReply("test-uuid2");

  EXPECT_EQ(BackupStatusReply::FAILED, reply1->status());
  EXPECT_EQ(BackupStatusReply::FINISHED, reply2->status());
}

} // namespace
