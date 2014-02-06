#include "block_device/block_device_factory.h"

#include <memory>
#include <string>

#include <blkid/blkid.h>
#include <glog/logging.h>
#include <unistd.h>
#include <linux/limits.h>

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

std::string GetPath(std::string uuid) {
  char real_path_buf[PATH_MAX];
  char *ret = realpath(("/dev/disk/by-uuid/" + uuid).c_str(), real_path_buf);
  if (ret == NULL) {
    PLOG(ERROR) << "Couldn't find UUID '" << uuid << "'";
    throw BlockDeviceException("Unable to find block device for UUID");
  }
  return std::string(real_path_buf);
}
}

namespace datto_linux_client {

std::shared_ptr<MountableBlockDevice>
BlockDeviceFactory::CreateMountableBlockDevice(std::string uuid) {
  std::shared_ptr<MountableBlockDevice> block_dev;

  std::string path = GetPath(uuid);
  std::string fs = GetFilesystem(path);

  if (fs == "ext4" || fs == "ext3") {
    block_dev = std::make_shared<ExtMountableBlockDevice>(path, false);
  } else if (fs == "ext2") {
    block_dev = std::make_shared<ExtMountableBlockDevice>(path, true);
  } else if (fs == "xfs") {
    block_dev = std::make_shared<XfsMountableBlockDevice>(path);
  } else {
    LOG(ERROR) << "Unknown filesystem '" << fs << "' for " << path;
    throw BlockDeviceException("Unknown filesystem");
  }
  return block_dev;
}

std::shared_ptr<RemoteBlockDevice>
BlockDeviceFactory::CreateRemoteBlockDevice(std::string hostname,
                                            uint16_t port_num) {
  return std::make_shared<NbdBlockDevice>(hostname, port_num);
}

} // datto_linux_client
