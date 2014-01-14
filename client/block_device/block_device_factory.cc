#include "block_device/block_device_factory.h"

#include <memory>
#include <string>

#include <blkid/blkid.h>
#include <glog/logging.h>

#include "block_device/nbd_block_device.h"
#include "block_device/ext_mountable_block_device.h"
#include "block_device/xfs_mountable_block_device.h"

namespace {

using datto_linux_client::BlockDeviceException;

std::string GetFilesystem(std::string path) {
  char *fs = ::blkid_get_tag_value(NULL, "TYPE", path.c_str());
  if (!fs) {
    LOG(ERROR) << "Couldn't get \"TYPE\" tag for " << path;
    throw BlockDeviceException("Unable to detect filesystem");
  }
  std::string fs_str(fs);
  free(fs);
  return fs_str;
}

}

namespace datto_linux_client {

std::unique_ptr<MountableBlockDevice>
BlockDeviceFactory::CreateMountableBlockDevice(std::string path) {
  std::unique_ptr<MountableBlockDevice> block_dev;

  std::string fs = GetFilesystem(path);
  if (fs == "ext4" || fs == "ext3" || fs == "ext2") {
    block_dev = std::unique_ptr<MountableBlockDevice>(
        new ExtMountableBlockDevice(path));
  } else if (fs == "xfs") {
    block_dev = std::unique_ptr<MountableBlockDevice>(
        new XfsMountableBlockDevice(path));
  } else {
    LOG(ERROR) << "Unknown filesystem '" << fs << "' for " << path;
    throw BlockDeviceException("Unknown filesystem");
  }
  return block_dev;
}

std::unique_ptr<RemoteBlockDevice>
BlockDeviceFactory::CreateRemoteBlockDevice(std::string hostname,
                                            uint16_t port_num) {
  auto block_dev = std::unique_ptr<RemoteBlockDevice>(
      new NbdBlockDevice(hostname, port_num));

  return block_dev;
}

} // datto_linux_client
