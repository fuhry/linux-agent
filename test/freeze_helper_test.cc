#include "freeze_helper/freeze_helper.h"

#include "block_device/mountable_block_device.h"

#include <unistd.h>
#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

using datto_linux_client::MountableBlockDevice;
using datto_linux_client::FreezeHelper;
using datto_linux_client::SectorSet;

class MockMountableBlockDevice : public MountableBlockDevice {
 public:
  MOCK_METHOD0(Freeze, void());
  MOCK_METHOD0(Thaw, void());
  MOCK_METHOD0(GetInUseSectors, std::shared_ptr<const SectorSet>());
};

TEST(FreezeHelperTest, Constructor) {
  MockMountableBlockDevice dev;
  FreezeHelper fh(dev, 1000);
}

TEST(FreezeHelperTest, DoesFreeze) {
  MockMountableBlockDevice dev;
  EXPECT_CALL(dev, Freeze());
  EXPECT_CALL(dev, Thaw());
  FreezeHelper fh(dev, 1000);

  fh.BeginRequiredFreezeBlock();
}

TEST(FreezeHelperTest, DoesUnfreeze) {
  MockMountableBlockDevice dev;
  EXPECT_CALL(dev, Freeze());
  EXPECT_CALL(dev, Thaw());
  FreezeHelper fh(dev, 500);

  fh.BeginRequiredFreezeBlock();
  sleep(1);
  bool was_frozen = fh.EndRequiredFreezeBlock();
  EXPECT_FALSE(was_frozen);
}

TEST(FreezeHelperTest, DoesKeepFrozen) {
  MockMountableBlockDevice dev;
  EXPECT_CALL(dev, Freeze());
  EXPECT_CALL(dev, Thaw());
  FreezeHelper fh(dev, 2000);

  fh.BeginRequiredFreezeBlock();
  usleep(500000);
  fh.EndRequiredFreezeBlock();

  fh.BeginRequiredFreezeBlock();
  usleep(500000);
  bool was_frozen = fh.EndRequiredFreezeBlock();
  EXPECT_TRUE(was_frozen);
}

}
