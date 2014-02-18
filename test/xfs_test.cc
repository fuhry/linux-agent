#include "fs_parsing/xfs_mountable_block_device.h"

#include <gtest/gtest.h>
#include <glog/logging.h>

#include "block_device/mountable_block_device.h"
#include "unsynced_sector_manager/sector_set.h"
#include "unsynced_sector_manager/sector_interval.h"
#include "test/loop_device.h"

#include <memory>
#include <fstream>
#include <sstream>

namespace {

using datto_linux_client::XfsMountableBlockDevice;
using datto_linux_client::SectorSet;
using datto_linux_client::SectorInterval;
using datto_linux_client_test::LoopDevice;

TEST(XfsTest, Dummy) {
  LoopDevice loop_dev;
  loop_dev.FormatAsXfs();

  XfsMountableBlockDevice bdev(loop_dev.path());

  SectorSet actual = *bdev.GetInUseSectors();

  LOG(INFO) << actual;
}

// TODO More tests

} // namespace
