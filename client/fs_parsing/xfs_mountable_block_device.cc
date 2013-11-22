#include "fs_parsing/xfs_mountable_block_device.h"

#include <regex>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <glog/logging.h>
#include "xfs/xfs.h"

#include "unsynced_sector_manager/sector_interval.h"

namespace {
auto BLOCK_SIZE_REGEX = std::regex("^blocksize = ([0-9]+$)");
auto AG_BLOCKS_REGEX = std::regex("^agblocks = ([0-9]+$)");
auto FREE_LIST_REGEX = std::regex("^ +([0-9]+) +([0-9])+ +([0-9])+ *$");

void CheckMatches(const std::smatch &matches, int expected_size) {
  if (matches.size() != expected_size) {
    LOG(ERROR) << "Bad number of matches";
    // TODO Better exception
    throw std::runtime_error("Unexpected line");
  }
}

}

namespace datto_linux_client {

XfsMountableBlockDevice::XfsMountableBlockDevice(std::string a_path)
    : MountableBlockDevice(a_path) { }

std::unique_ptr<const SectorSet> XfsMountableBlockDevice::GetInUseSectors() {

  // we are going to subtrace the free blocks later, so start with everything
  // in use
  auto in_use_sectors = std::unique_ptr<SectorSet>(new SectorSet());
  *in_use_sectors += SectorInterval(0, DeviceSizeBytes() / 512);

  // open a pipe for the xfs_db process
  int pipefd[2];
  if (pipe(pipefd)) {
    PLOG(ERROR) << "Unable to open pipe for xfs_db";
    // TODO Custom exception
    throw std::runtime_error("fill me out");
  }

  // run xfs_db
  pid_t fork_ret = fork();
  if (fork_ret < 0) {
    // error
  } else if (fork_ret == 0) {
    // child
    // close read end of pipe
    close(pipefd[0]);
    // stdout goes to pipe
    dup2(pipefd[1], 1);
    // stderr goes to pipe
    dup2(pipefd[1], 2);
    execlp("xfs_db", "-r", path_.c_str(),
           "-c", "sb",
           "-c", "print blocksize agblock",
           "-c", "freesp -d");
    PLOG(ERROR) << "execlp for xfs_db returned";
    _exit(1);
  } 
  // parent

  // close write end of pipe
  close(pipefd[1]);
  // TODO error check
  FILE *xfs_db_out = fdopen(pipefd[0], "r");

  // read from pipe
  char buf[1024];
  int block_size = -1;
  int ag_blocks = -1;

  while (fgets(buf, 1024, xfs_db_out)) {
    std::string line(buf);
    std::smatch matches;
    // block_size
    if (std::regex_match(line, matches, BLOCK_SIZE_REGEX)) {
      CheckMatches(matches, 2);
      block_size = std::stoi(matches[1]);
    // ag_blocks
    } else if (std::regex_match(line, matches, AG_BLOCKS_REGEX)) {
      CheckMatches(matches, 2);
      ag_blocks = std::stoi(matches[1]);
    // remove free blocks from in_use_sectors
    } else if (std::regex_match(line, matches, FREE_LIST_REGEX)) {
      CheckMatches(matches, 4);

      if (block_size == -1 || ag_blocks == -1) {
        LOG(ERROR) << "Got lines in unexpected order";
        // TODO Better exception
        throw std::runtime_error("Unexpected line");
      }

      uint64_t start_block = (std::stoll(matches[1]) * ag_blocks) + 
                              std::stoll(matches[2]);
      uint64_t start_sector = start_block * block_size / 512; 
      uint64_t num_sectors = std::stoll(matches[3]) * block_size / 512;
      *in_use_sectors -= SectorInterval(start_sector,
                                        start_sector + num_sectors);
    }
    // ignore any non-matching lines
  }

  int status;
  if (waitpid(fork_ret, &status, 0) == -1) {
    PLOG(ERROR) << "Error waiting for child";
    // TODO throw
  }

  if (!WIFEXITED(status)) {
    LOG(ERROR) << "xfs_db exited non-normally";
    // TODO throw
  } else if (WEXITSTATUS(status) != 0) {
    LOG(ERROR) << "xfs_db returned with bad status " << WEXITSTATUS(status);
    // TODO throw
  }

  return std::move(in_use_sectors);
}

} // datto_linux_client
