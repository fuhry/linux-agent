#ifndef DATTO_CLIENT_BACKUP_BACKUP_H_
#define DATTO_CLIENT_BACKUP_BACKUP_H_

#include <atomic>
#include <memory>

#include "block_device/block_device.h"
#include "block_device/mountable_block_device.h"
#include "request_listener/reply_channel.h"
#include "unsynced_sector_manager/unsynced_sector_store.h"

namespace datto_linux_client {

enum BackupStatus {
  NOT_STARTED = 0,
  PREPARING,
  COPYING,
  CLEANING_UP,
  FINISHED,
  FAILED,
};

class Backup {
 public:
  Backup(std::shared_ptr<MountableBlockDevice> source_device,
         std::shared_ptr<UnsyncedSectorStore> source_sector_store,
         std::shared_ptr<BlockDevice> destination_device,
         std::shared_ptr<ReplyChannel> reply_channel);

  void DoBackup();
  void Stop();

  BackupStatus status() {
    return status_;
  }

  virtual ~Backup();

  Backup(const Backup &) = delete;
  Backup& operator=(const Backup &) = delete;
 protected:
  virtual void Prepare() = 0;
  virtual void Copy();
  virtual void Cleanup() = 0;

  std::shared_ptr<MountableBlockDevice> source_device_;
  std::shared_ptr<UnsyncedSectorStore> source_sector_store_;
  std::shared_ptr<BlockDevice> destination_device_;
  std::shared_ptr<ReplyChannel> reply_channel_;

 private:
  BackupStatus status_;
  std::atomic<bool> do_stop_;
};

} // datto_linux_client

#endif  // DATTO_CLIENT_BACKUP_BACKUP_H_
