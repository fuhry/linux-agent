#include "device_synchronizer/device_synchronizer.h"

#include <stdint.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <glog/logging.h>

#include "device_synchronizer/device_synchronizer_exception.h"
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
                       int block_size_bytes) {
  char buf[block_size_bytes];

  DLOG_EVERY_N(INFO, 500) << "Copying block " << google::COUNTER;
  ssize_t total_copied = 0;
  // Loop until we copy a full block
  do {
    ssize_t bytes_read = read(source_fd, buf, block_size_bytes);
    if (bytes_read == -1) {
      PLOG(ERROR) << "Error while reading from source";
      throw DeviceSynchronizerException("Error reading from source");
    } else if (bytes_read == 0) {
      // No reads means we are done
      PLOG(INFO) << "No bytes read";
      break;
    }

    // Loop until we clear the write buffer
    ssize_t count_written = 0;
    do {
      ssize_t bytes_written = write(destination_fd, buf, bytes_read);
      if (bytes_written == -1) {
        PLOG(ERROR) << "Error while writing to destination";
        throw DeviceSynchronizerException("Error writing to destination");
      } else if (bytes_written == 0) {
        break;
      }
      count_written += bytes_written;
    } while (count_written != bytes_read); // clear write buffer

    // bytes_read == count_written here
    total_copied += count_written;

  } while (total_copied != block_size_bytes); // copy full block
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

  CHECK(sector_manager_->IsTracing(*source_device_));

  uint64_t total_bytes_sent = 0;
  int source_fd = source_device_->Open();
  int destination_fd = destination_device_->Open();

  int block_size_bytes = source_device_->BlockSizeBytes();
  int sectors_per_block = block_size_bytes / SECTOR_SIZE;
  DLOG(INFO) << "Sectors per block: " << sectors_per_block;

  auto source_store = sector_manager_->GetStore(*source_device_);

  time_t flush_time = 0;
  time_t freeze_time = 0;

  bool was_done = false;

  while (!coordinator->IsCancelled()) {
    uint64_t unsynced_sector_count = source_store->UnsyncedSectorCount();

    // Let the event handler know how much is left
    count_handler->UpdateUnsyncedCount(unsynced_sector_count * SECTOR_SIZE);

    if (flush_time > 0 && unsynced_sector_count == 0) {
      LOG(INFO) << "Sync complete";
      if (freeze_time > 0) {
        source_device_->Thaw();
        freeze_time = 0;
      }
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

    // Thaw if it's been more than a couple seconds since we froze
    if (freeze_time > 0 && time(NULL) - freeze_time > SECONDS_TO_FREEZE) {
      LOG(WARNING) << "Unfreezing due to time";
      source_device_->Thaw();
      freeze_time = 0;
    }

    // to_sync_interval is sectors, not blocks
    SectorInterval to_sync_interval =
      source_store->GetContinuousUnsyncedSectors();

    DLOG(INFO) << "Cardinality is: "
               << boost::icl::cardinality(to_sync_interval);
    source_store->MarkToSyncInterval(to_sync_interval);

    off_t seek_pos = to_sync_interval.lower() * SECTOR_SIZE;
    seek_devices_to(seek_pos, source_fd, destination_fd);

    // Loop until we copy all of the blocks of the sector interval
    DLOG(INFO) << "Syncing interval: " << to_sync_interval;
    for (uint64_t i = 0;
        i < boost::icl::cardinality(to_sync_interval);
        i += sectors_per_block) {
      copy_block(source_fd, destination_fd, block_size_bytes);
    }
    DLOG(INFO) << "Finished copying interval " << to_sync_interval;

    total_bytes_sent +=
        boost::icl::cardinality(to_sync_interval) * SECTOR_SIZE;
    count_handler->UpdateSyncedCount(total_bytes_sent);

    // Update unsynced sector count after interval was synced
    unsynced_sector_count = source_store->UnsyncedSectorCount();

    // If there is under 1MB left, freeze and flush the filesystem so things
    // are consistent while it wraps up
    if (unsynced_sector_count < ONE_MEGABYTE / SECTOR_SIZE) {
      if (!freeze_time) {
        try {
          source_device_->Freeze();
          freeze_time = time(NULL);
        } catch (const BlockDeviceException &e) {
          LOG(WARNING) << "Unable to freeze "
                       << source_device_->path() << ": " << e.what();
        }
      }
      if (time(NULL) - flush_time > SECONDS_BETWEEN_FLUSHES) {
        source_device_->Flush();
        sector_manager_->FlushTracer(*source_device_);
        flush_time = time(NULL);
      }
    }
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
