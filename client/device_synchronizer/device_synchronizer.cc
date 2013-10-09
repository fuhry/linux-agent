#include "device_synchronizer/device_synchronizer.h"
#include "device_synchronizer/device_synchronizer_exception.h"
#include "unsynced_sector_tracker/sector_interval.h"
#include <glog/logging.h>

namespace {
int MAX_SIZE_WORK_LEFT_HISTORY = 3 * 60;

// Keep this simple for now
inline bool should_continue(const std::vector<uint64_t> &work_left_history) {
  // If we have less than a minute of data, don't do anything
  if (work_left_history.size() < 60) {
    return true;
  }
  return work_left_history.back() < work_left_history.front();
}

inline void copy_block(int source_fd, int destination_fd,
                       int block_size_bytes) {
  ssize_t total_copied = 0;
  // Loop until we copy a full block
  do {
    ssize_t bytes_read = read(source_fd, buf, block_size_bytes);
    if (bytes_read == -1) {
      PLOG(ERROR) << "Error while reading from source";
      throw DeviceSynchronizerException("Error reading from source");
    } else if (bytes_read == 0) {
      // TODO No reads means we are done (I think?)
      break;
    }
    // Loop until we clear the write buffer
    ssize_t total_written = 0;
    do {
      ssize_t bytes_written = write(destination_fd, buf, bytes_read);

      if (bytes_written == -1) {
        PLOG(ERROR) << "Error while writing to destination";
        throw DeviceSynchronizerException("Error writing to destination");
      } else if (bytes_written == 0) {
        break;
      }
      total_written += bytes_written;
    } while(bytes_written != bytes_read); // clear write buffer
  } while (total_copied != block_size_bytes); // copy full block
}
} // unnamed namespace

namespace datto_linux_client {

DeviceSynchronizer::DeviceSynchronizer(
    std::unique_ptr<MountableBlockDevice> source_device,
    std::shared_ptr<UnsyncedSectorTracker> sector_tracker,
    std::unique_ptr<BlockDevice> destination_device,
    std::shared_ptr<ReplyChannel> reply_channel)
    : should_stop_(false),
      succeeded_(false),
      source_device_(source_device_),
      destination_device_(destination_device),
      sector_tracker_(sector_tracker),
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

  if (sector_tracker_->UnsyncedSectorCount() == 0) {
    throw DeviceSynchronizerException("The source device is already synced");
  }

  if (!reply_channel_->IsAvailable()) {
    throw DeviceSynchronizerException("Reply channel is unavailable,"
                                      " stopping");
  }
}

DeviceSynchronizer::StartSync() {
  // We need to be careful as we are starting a non-trival thread.
  // If it throws an uncaught exception everything goes down
  // without cleaning up (no destructors!), which isn't okay.
  sync_thread_ = std::thread([&]() {
    try {
      int source_fd = source_device_->Open();
      int destination_fd = destination_device_->Open();

      int block_size_bytes = source_device_->BlockSizeBytes();
      int sectors_per_block = SECTOR_SIZE / block_size_bytes;

      char buf[block_size_bytes];
      // We keep track of how much work is left after we sent it, so that
      // way we can detect if we are in a situation where data is changing
      // after than we can back it up
      std::vector<uint64_t> work_left_history;

      uint64_t total_blocks_copied = 0;
      time_t last_history = 0;

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
          sector_tracker_->GetContinuousUnsyncedSectors();

        // If the only interval is size zero, we are done
        if (to_sync_interval.cardinality() == 0) {
          succeeded_ = true;
          break;
        }

        off_t seek_pos = to_sync_interval.lower() * SECTOR_SIZE
        int seek_ret = lseek(source_fd, seek_pos, SEEK_SET);

        if (seek_ret == -1) {
          PLOG(ERROR) << "Error while seeking";
          throw DeviceSynchronizerException("Unable to seek source");
        }

        seek_ret = lseek(destination_fd,
                         to_sync_interval.lower() * SECTOR_SIZE, SEEK_SET);

        if (seek_ret == -1) {
          PLOG(ERROR) << "Error while seeking";
          throw DeviceSynchronizerException("Unable to seek destination");
        }

        // Loop until we copy all of the blocks of the sector interval
        for (uint64_t i = 0;
             i < to_sync_interval.cardinality;
             i += sectors_per_block) {

          copy_block(source_fd, destination_fd, block_size_bytes);

          // Get one history entry per second
          time_t now = time(NULL);
          if (now > last_history + 1) {
            if (now > last_history + 2) {
              LOG(WARN) << "Writing a block took more than a second";
            }
            uint64_t unsynced = sector_tracker_->UnsyncedSectorCount();
            work_left_history.push_back(unsynced);
            last_history = now;
          }
        } // copy all blocks in interval
      } while (!should_stop);
    } catch (const std::runtime_error &e) {
      LOG(ERROR) << "Error while performing sync. Stopping. " << e.what();
    }

    source_device_->Close();
    destination_device_->Close();
  }); // end thread
}
}
