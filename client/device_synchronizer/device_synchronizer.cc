#include "device_synchronizer/device_synchronizer.h"

#include <stdint.h>
#include <unistd.h>
#include <time.h>

#include <glog/logging.h>

#include "device_synchronizer/device_synchronizer_exception.h"
#include "unsynced_sector_manager/sector_interval.h"

namespace {

using ::datto_linux_client::DeviceSynchronizerException;

int SECTOR_SIZE = 512;
std::vector<uint64_t>::size_type MAX_SIZE_WORK_LEFT_HISTORY = 3 * 60;

// TODO Make this slightly more intelligent
// Keep this simple for now
inline bool should_continue(const std::vector<uint64_t> &work_left_history) {
  // If we have less than a minute of data, don't do anything
  if (work_left_history.size() < 60) {
    return true;
  }
  DLOG(INFO) << "Determining if the process should stop";
  DLOG(INFO) << "Back: " << work_left_history.back()
             << " Front: " << work_left_history.front();
  return true;
}

// It is up to the caller to ensure buf is large enough
inline void copy_block(int source_fd, int destination_fd,
                       void *buf, int block_size_bytes) {
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
    std::shared_ptr<MountableBlockDevice> source_device,
    std::shared_ptr<UnsyncedSectorManager> source_unsynced_manager,
    std::shared_ptr<BlockDevice> destination_device,
    std::shared_ptr<BackupEventHandler> event_handler)
    : source_device_(source_device),
      source_unsynced_manager_(source_unsynced_manager),
      destination_device_(destination_device),
      event_handler_(event_handler) {

  if (source_device_->major() == destination_device_->major()
      && source_device_->minor() == destination_device_->minor()) {
    LOG(ERROR) << "Major: " << source_device_->major();
    LOG(ERROR) << "Minor: " << source_device_->minor();
    throw DeviceSynchronizerException("Refusing to synchronize a device"
                                      " with itself");
  }

  if (source_device_->DeviceSizeBytes() !=
      destination_device_->DeviceSizeBytes()) {
    LOG(ERROR) << "Source size: "
               << source_device_->DeviceSizeBytes();
    LOG(ERROR) << "Destination size: "
               << destination_device_->DeviceSizeBytes();
    throw DeviceSynchronizerException("Source and destination device have"
                                      " different sizes");
  }

  uint64_t unsynced_count =
      source_unsynced_manager_->store()->UnsyncedSectorCount();

  event_handler_->UpdateUnsyncedCount(unsynced_count);

  if (unsynced_count == 0) {
    throw DeviceSynchronizerException("The source device is already synced");
  }
}

void DeviceSynchronizer::DoSync(std::shared_ptr<CancellationToken> cancel_token) {
  LOG(INFO) << "Starting sync ";
  int source_fd = source_device_->Open();
  int destination_fd = destination_device_->Open();

  int block_size_bytes = source_device_->BlockSizeBytes();
  int sectors_per_block = block_size_bytes / SECTOR_SIZE;

  char buf[block_size_bytes];

  // We keep track of how much work is left after we sent it, so that way
  // we can detect if we are in a situation where data is changing after
  // than we can back it up
  //
  // The most recent data is at the end. This means trimming is probably
  // the most expensive operation, as the old data at the front needs to be
  // removed. I doubt this will be an issue though, vectors are extremely
  // efficient in C++
  std::vector<uint64_t> work_left_history;

  time_t last_history = time(NULL);

  auto source_store = source_unsynced_manager_->store();

  time_t freeze_time = 0;

  while (!cancel_token->ShouldCancel()) {
    // Unthaw if it's been more than a couple seconds
    if (freeze_time && (time(NULL) - freeze_time > 2)) {
      PLOG(WARNING) << "Unfreezing due to time";
      source_device_->Thaw();
      freeze_time = 0;
    }

    // Let the event handler know how much is left
    uint64_t unsynced_count = source_store->UnsyncedSectorCount();
    event_handler_->UpdateUnsyncedCount(unsynced_count);

    // Trim history if it gets too big
    if (work_left_history.size() > MAX_SIZE_WORK_LEFT_HISTORY) {
      DLOG(INFO) << "Trimming history";
      int num_to_trim = work_left_history.size() -
        MAX_SIZE_WORK_LEFT_HISTORY;
      work_left_history.erase(work_left_history.begin(),
          work_left_history.begin() + num_to_trim);
    }

    if (!should_continue(work_left_history)) {
      DLOG(WARNING) << "Giving up";
      throw DeviceSynchronizerException("Unable to copy data faster"
          " than it is changing");
    }

    // to_sync_interval is sectors, not blocks
    SectorInterval to_sync_interval =
      source_store->GetContinuousUnsyncedSectors();

    DLOG(INFO) << "Syncing interval: " << to_sync_interval;

    // If the only interval is size zero, we are done
    if (boost::icl::cardinality(to_sync_interval) == 0) {
      if (!freeze_time) {
        source_device_->Freeze();
        freeze_time = time(NULL);
        source_device_->Flush();
        source_unsynced_manager_->FlushTracer();
        continue;
      }

      source_device_->Thaw();
      LOG(INFO) << "Got size 0 interval (process is finished)";
      break;
    }

    off_t seek_pos = to_sync_interval.lower() * SECTOR_SIZE;

    off_t seek_ret = lseek(source_fd, seek_pos, SEEK_SET);

    if (seek_ret == -1) {
      PLOG(ERROR) << "Error while seeking source to " << seek_pos;
      throw DeviceSynchronizerException("Unable to seek source");
    }

    seek_ret = lseek(destination_fd, seek_pos, SEEK_SET);

    if (seek_ret == -1) {
      PLOG(ERROR) << "Error while seeking destination to " << seek_pos;
      throw DeviceSynchronizerException("Unable to seek destination");
    }

    DLOG(INFO) << "Marking interval " << to_sync_interval;
    DLOG(INFO) << "Cardinality is: "
      << boost::icl::cardinality(to_sync_interval);
    DLOG(INFO) << "Sectors per block: " << sectors_per_block;
    source_store->MarkToSyncInterval(to_sync_interval);
    // Loop until we copy all of the blocks of the sector interval
    for (uint64_t i = 0;
        i < boost::icl::cardinality(to_sync_interval);
        i += sectors_per_block) {

      copy_block(source_fd, destination_fd, buf, block_size_bytes);

      // Get one history entry per second
      time_t now = time(NULL);
      if (now > last_history + 1) {
        if (now > last_history + 2) {
          LOG(WARNING) << "Writing a block took more than a second";
        }
        uint64_t unsynced = source_store->UnsyncedSectorCount();
        work_left_history.push_back(unsynced);
        last_history = now;
      }
    } // copy all blocks in interval
    DLOG(INFO) << "Finished copying interval " << to_sync_interval;
  }
  DLOG(INFO) << "Sync completed";
  source_device_->Close();
  destination_device_->Close();
}

DeviceSynchronizer::~DeviceSynchronizer() {
  DLOG(INFO) << "Closing source and destination device";
  source_device_->Close();
  destination_device_->Close();
}

} // datto_linux_client
