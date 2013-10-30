#include "device_synchronizer/device_synchronizer.h"
#include "device_synchronizer/device_synchronizer_exception.h"
#include "unsynced_sector_manager/sector_interval.h"
#include <glog/logging.h>

namespace {

using ::datto_linux_client::DeviceSynchronizerException;
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
  return work_left_history.back() < work_left_history.front();
}

// It is up to the caller to ensure buf is large enough
inline void copy_block(int source_fd, int destination_fd,
                       void *buf, int block_size_bytes) {
  DLOG_EVERY_N(INFO, 1) << "Copying block " << google::COUNTER;
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
    } while(count_written != bytes_read); // clear write buffer

    total_copied += count_written;

  } while (total_copied != block_size_bytes); // copy full block
}
} // unnamed namespace

namespace datto_linux_client {

DeviceSynchronizer::DeviceSynchronizer(
    std::shared_ptr<MountableBlockDevice> source_device,
    std::shared_ptr<UnsyncedSectorStore> sector_store,
    std::shared_ptr<BlockDevice> destination_device,
    std::shared_ptr<ReplyChannel> reply_channel)
    : should_stop_(false),
      succeeded_(false),
      done_(false),
      source_device_(source_device),
      sector_store_(sector_store),
      destination_device_(destination_device),
      reply_channel_(reply_channel) {

  if (source_device_->major() == destination_device_->major()
      && source_device_->minor() == destination_device_->minor()) {
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

  if (sector_store_->UnsyncedSectorCount() == 0) {
    throw DeviceSynchronizerException("The source device is already synced");
  }

  if (!reply_channel_->IsAvailable()) {
    throw DeviceSynchronizerException("Reply channel is unavailable,"
                                      " stopping");
  }
}

void DeviceSynchronizer::StartSync() {
  // We need to be careful as we are starting a non-trival thread.
  // If it throws an uncaught exception everything goes down
  // without cleaning up (no destructors!), which isn't okay.
  sync_thread_ = std::thread([&]() {
    try {
      LOG(INFO) << "Starting sync thread";
      int source_fd = source_device_->Open();
      int destination_fd = destination_device_->Open();

      int block_size_bytes = source_device_->BlockSizeBytes();
      int sectors_per_block = block_size_bytes / SECTOR_SIZE;

      char buf[block_size_bytes];
      // We keep track of how much work is left after we sent it, so that
      // way we can detect if we are in a situation where data is changing
      // after than we can back it up
      std::vector<uint64_t> work_left_history;

      time_t last_history = time(NULL);

      do {
        // Trim history if it gets too big
        if (work_left_history.size() > MAX_SIZE_WORK_LEFT_HISTORY) {
          int num_to_trim = work_left_history.size() -
                            MAX_SIZE_WORK_LEFT_HISTORY;
          work_left_history.erase(work_left_history.begin(),
                                  work_left_history.begin() + num_to_trim);
        }

        if (!should_continue(work_left_history)) {
          throw DeviceSynchronizerException("Unable to copy data faster"
                                            " than it is changing");
        }

        // to_sync_interval is sectors, not blocks
        SectorInterval to_sync_interval =
            sector_store_->GetContinuousUnsyncedSectors();

        // If the only interval is size zero, we are done
        if (boost::icl::cardinality(to_sync_interval) == 0) {
          succeeded_ = true;
          break;
        }

        off_t seek_pos = to_sync_interval.lower() * SECTOR_SIZE;
        int seek_ret = lseek(source_fd, seek_pos, SEEK_SET);

        if (seek_ret == -1) {
          PLOG(ERROR) << "Error while seeking";
          throw DeviceSynchronizerException("Unable to seek source");
        }

        seek_ret = lseek(destination_fd,
                         to_sync_interval.lower() * SECTOR_SIZE,
                         SEEK_SET);

        if (seek_ret == -1) {
          PLOG(ERROR) << "Error while seeking";
          throw DeviceSynchronizerException("Unable to seek destination");
        }

        DLOG(INFO) << "Marking " << to_sync_interval.lower() << " : "
                   << to_sync_interval.upper();
        DLOG(INFO) << "Cardinality is: "
                   << boost::icl::cardinality(to_sync_interval);
        DLOG(INFO) << "Sectors per block: " << sectors_per_block;
        sector_store_->MarkToSyncInterval(to_sync_interval);
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
            uint64_t unsynced = sector_store_->UnsyncedSectorCount();
            work_left_history.push_back(unsynced);
            last_history = now;
          }
        } // copy all blocks in interval
      } while (!should_stop_);
    } catch (const std::runtime_error &e) {
      LOG(ERROR) << "Error while performing sync. Stopping. " << e.what();
    }

    source_device_->Close();
    destination_device_->Close();
    done_ = true;
  }); // end thread

  sync_thread_.detach();
}
}
