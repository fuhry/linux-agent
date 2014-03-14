#include "device_synchronizer/device_synchronizer.h"

#include <stdint.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <glog/logging.h>

#include "device_synchronizer/device_synchronizer_exception.h"
#include "freeze_helper/freeze_helper.h"
#include "unsynced_sector_manager/sector_interval.h"

namespace {

using ::datto_linux_client::DeviceSynchronizerException;

uint32_t SECTOR_SIZE = 512;
uint32_t ONE_MEGABYTE = 1024 * 1024;

time_t SECONDS_BETWEEN_FLUSHES = 2;
time_t SECONDS_TO_FREEZE = 2;

void seek_devices_to(off_t seek_pos, int source_fd, int dest_fd) {
  off_t seek_ret = lseek(source_fd, seek_pos, SEEK_SET);
  if (seek_ret == -1) {
    PLOG(ERROR) << "Error while seeking source to " << seek_pos;
    throw DeviceSynchronizerException("Unable to seek source");
  }
  seek_ret = lseek(dest_fd, seek_pos, SEEK_SET);
  if (seek_ret == -1) {
    PLOG(ERROR) << "Error while seeking destination to " << seek_pos;
    throw DeviceSynchronizerException("Unable to seek destination");
  }
}


inline void copy_block(int source_fd, int destination_fd,
                       ssize_t block_size_bytes, off_t offset) {
  char buf[block_size_bytes];

  DLOG_EVERY_N(INFO, 500) << "Copying block " << google::COUNTER;

  ssize_t bytes_read = pread(source_fd, buf, block_size_bytes, offset);
  if (bytes_read == -1) {
    PLOG(ERROR) << "Error while reading from source";
    throw DeviceSynchronizerException("Error reading from source");
  } else if (bytes_read != block_size_bytes) {
    PLOG(INFO) << "Expected to read " << block_size_bytes
      << ". Got " << bytes_read;
    throw DeviceSynchronizerException("Unexpected read result");
  }

  ssize_t bytes_written = pwrite(destination_fd, buf, bytes_read, offset);
  if (bytes_written == -1) {
    PLOG(ERROR) << "Error while writing to destination";
    throw DeviceSynchronizerException("Error writing to destination");
  } else if (bytes_written != block_size_bytes) {
    PLOG(INFO) << "Expected to write " << block_size_bytes
      << ". Got " << bytes_written;
    throw DeviceSynchronizerException("Unexpected read result");
  }
}
} // unnamed namespace

namespace datto_linux_client {

DeviceSynchronizer::DeviceSynchronizer(
    std::shared_ptr<MountableBlockDevice> source_device_a,
    std::shared_ptr<UnsyncedSectorManager> sector_manager_a,
    std::shared_ptr<BlockDevice> destination_device_a)
    : source_device_(source_device_a),
      sector_manager_(sector_manager_a),
      destination_device_(destination_device_a) {

  if (source_device_->dev_t() == destination_device_->dev_t()) {
    LOG(ERROR) << "Attempt to synchronize a device with itself";
    LOG(ERROR) << "device: " << ::minor(source_device_->dev_t())
               << ":" << major(source_device_->dev_t());
    throw DeviceSynchronizerException("Refusing to synchronize a device"
                                      " with itself");
  }

  if (source_device_->DeviceSizeBytes() >
      destination_device_->DeviceSizeBytes()) {
    LOG(ERROR) << "Source size: "
               << source_device_->DeviceSizeBytes();
    LOG(ERROR) << "Destination size: "
               << destination_device_->DeviceSizeBytes();
    throw DeviceSynchronizerException("Destination device is too small");
  }
}

void DeviceSynchronizer::DoSync(
    std::shared_ptr<BackupCoordinator> coordinator,
    std::shared_ptr<SyncCountHandler> count_handler) {
  LOG(INFO) << "Starting sync ";

  uint64_t total_bytes_sent = 0;
  int source_fd = source_device_->Open();
  int destination_fd = destination_device_->Open();
  FreezeHelper freeze_helper(*source_device_, SECONDS_TO_FREEZE * 1000);

  const int block_size_bytes = source_device_->BlockSizeBytes();
  const int sectors_per_block = block_size_bytes / SECTOR_SIZE;
  DLOG(INFO) << "Sectors per block: " << sectors_per_block;

  auto source_store = sector_manager_->GetStore(*source_device_);

  time_t flush_time = 0;
  bool was_done = false;

  while (!coordinator->IsCancelled()) {
    uint64_t unsynced_sector_count = source_store->UnsyncedSectorCount();

    // If there is under 1MB left, freeze and flush the filesystem so things
    // are consistent while it wraps up
    if (unsynced_sector_count < ONE_MEGABYTE / SECTOR_SIZE) {
      if (time(NULL) - flush_time > SECONDS_BETWEEN_FLUSHES) {
        source_device_->Flush();
        flush_time = time(NULL);
      }

      // Update sync count after flush and during freeze
      freeze_helper.RunWhileFrozen([&]() {
        // Let the trace data hit
        sector_manager_->FlushTracer(*source_device_);
        unsynced_sector_count = source_store->UnsyncedSectorCount();
      });
    }

    // Let the event handler know how much is left
    count_handler->UpdateUnsyncedCount(unsynced_sector_count * SECTOR_SIZE);

    if (flush_time > 0 && unsynced_sector_count == 0) {
      LOG(INFO) << "Sync complete";
      if (!was_done) {
        coordinator->SignalFinished();
        was_done = true;
      }
      if (!coordinator->WaitUntilFinished(500)) {
        continue;
      }
      break;
    } else if (was_done) {
      was_done = false;
      coordinator->SignalMoreWorkToDo();
    }

    SectorInterval to_sync_interval;
    // to_sync_interval is sectors, not blocks
    bool is_volatile = source_store->GetInterval(&to_sync_interval,
                                                 time(NULL));

    DLOG(INFO) << "Cardinality is: "
               << boost::icl::cardinality(to_sync_interval);
    source_store->RemoveInterval(to_sync_interval);

    // Loop until we copy all of the blocks of the sector interval
    DLOG(INFO) << "Syncing interval: " << to_sync_interval;

    off_t offset = to_sync_interval.lower() * SECTOR_SIZE;
    for (uint64_t i = 0;
        i < boost::icl::cardinality(to_sync_interval);
        i += sectors_per_block) {

      auto copy_func = [&]() {
        copy_block(source_fd, destination_fd, block_size_bytes, offset);
      };

      if (is_volatile) {
        freeze_helper.RunWhileFrozen(copy_func);
      } else {
        copy_func();
      }
      offset += block_size_bytes;
    }
    DLOG(INFO) << "Finished copying interval " << to_sync_interval;

    total_bytes_sent +=
        boost::icl::cardinality(to_sync_interval) * SECTOR_SIZE;
    count_handler->UpdateSyncedCount(total_bytes_sent);
  }
  source_device_->Close();
  destination_device_->Close();
  DLOG(INFO) << "Sync completed";
}

DeviceSynchronizer::~DeviceSynchronizer() {
  DLOG(INFO) << "Closing source and destination device";
  source_device_->Close();
  destination_device_->Close();
}

} // datto_linux_client
