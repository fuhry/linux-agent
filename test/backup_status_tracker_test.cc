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

  handler->BackupInProgress();
  auto reply = tracker.GetReply("test-uuid");
  EXPECT_EQ(BackupStatusReply::IN_PROGRESS, reply->status());

  handler->BackupSucceeded();
  // Old reply should not have changed
  EXPECT_EQ(BackupStatusReply::IN_PROGRESS, reply->status());
  reply = tracker.GetReply("test-uuid");
  // New reply should have changed
  EXPECT_EQ(BackupStatusReply::SUCCEEDED, reply->status());
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
  handler2->BackupSucceeded();

  auto reply1 = tracker.GetReply("test-uuid1");
  auto reply2 = tracker.GetReply("test-uuid2");

  EXPECT_EQ(BackupStatusReply::FAILED, reply1->status());
  EXPECT_EQ(BackupStatusReply::SUCCEEDED, reply2->status());
}

} // namespace
