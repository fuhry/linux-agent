#include "block_device/mountable_block_device.h"

#include <blkid/blkid.h>
#include <fcntl.h>
#include <fstream>
#include <glog/logging.h>
#include <linux/fs.h>
#include <linux/limits.h>
#include <map>
#include <string>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "block_device/block_device_exception.h"

namespace {
// This function returns all device paths that are mounted
std::map<std::string, std::string> GetMountedDevices() {
  std::map<std::string, std::string> mounted_devices;

  std::ifstream mounts("/proc/mounts", std::ios::in);

  while (mounts.good()) {
    char line[1024];
    mounts.getline(line, PATH_MAX);
    std::string line_str(line);

    int first_space = line_str.find(' ');
    int second_space = line_str.find(' ', first_space + 1);
    std::string path = line_str.substr(0, first_space);
    VLOG(2) << "Path is: " << path;
    std::string mount_point = line_str.substr(first_space + 1,
                                              second_space - first_space - 1);

    char real_path_buf[PATH_MAX];
    if (realpath(path.c_str(), real_path_buf) == NULL) {
      int error = errno;
      if (error == ENOENT) {
        continue;
      } else if (error == EACCES) {
        continue;
      } else if (error == ENOTDIR) {
        continue;
      } else {
        PLOG(ERROR) << "Error during readlink";
        throw datto_linux_client::BlockDeviceException("readlink");
      }
    }
    std::string real_path = std::string(real_path_buf);

    // TODO this replace only handles the character ' ' in a path.
    // There are other characters that aren't as they seem (e.g. \),
    // and we should handle those too

    // 4 is the length of "\\040", 1 is the length of " "
    size_t space_pos = 0;
    while((space_pos = mount_point.find(4, space_pos))
        != std::string::npos) {
      mount_point.replace(space_pos, 4, " ");
      space_pos += 1;
    }

    VLOG(2) << "Adding " << real_path << " : " << mount_point;
    mounted_devices[real_path] = mount_point;
  }

  return mounted_devices;
}
} // unnamed namespace

namespace datto_linux_client {

MountableBlockDevice::MountableBlockDevice(std::string a_path)
    : BlockDevice(a_path),
      mount_file_descriptor_(-1),
      is_frozen_(false) { }

bool MountableBlockDevice::IsMounted() {
  // TODO These checks should be based on major/minor number, not on path.
  // mknod can be used to create "block special" type devices all over the
  // file system that all refer to the same block device but don't
  // show up as symbolic links
  auto mounted_devices = GetMountedDevices();
  return mounted_devices.find(path_) != mounted_devices.end();
}

std::string MountableBlockDevice::GetMountPoint() {
  auto mounted_devices = GetMountedDevices();
  auto mount_pair = mounted_devices.find(path_);
  if (mount_pair == mounted_devices.end()) {
    LOG(ERROR) << path_ << " is not mounted";
    throw BlockDeviceException("Block device is not mounted");
  }
  return mount_pair->second;
}

void MountableBlockDevice::Freeze() {
  int mount_fd = OpenMount();
  int ioctl_ret = ioctl(mount_fd, FIFREEZE, /* ignored */ 0);
  close(mount_fd);

  if (ioctl_ret) {
    PLOG(ERROR) << "Error during freeze.";
    throw BlockDeviceException("FIFREEZE");
  }

  is_frozen_ = true;
}

void MountableBlockDevice::Thaw() {
  int mount_fd = OpenMount();
  int ioctl_ret = ioctl(mount_fd, FITHAW, /* ignored */ 0);
  close(mount_fd);

  if (ioctl_ret) {
    PLOG(ERROR) << "Error during thaw.";
    throw BlockDeviceException("FITHAW");
  }

  is_frozen_ = false;
}

int MountableBlockDevice::OpenMount() {
  std::string mount_point = GetMountPoint();
  if ((mount_file_descriptor_ = open(mount_point.c_str(),
                                     O_DIRECTORY | O_RDONLY)) == -1) {
    PLOG(ERROR) << "error opening " << mount_point;
    throw BlockDeviceException("Opening mountpoint");
  }

  return mount_file_descriptor_;
}

void MountableBlockDevice::CloseMount() {
  close(mount_file_descriptor_);
}

std::string MountableBlockDevice::GetUuid() const {
  char *uuid = ::blkid_get_tag_value(NULL, "UUID", path_.c_str());
  if (!uuid) {
    LOG(ERROR) << "Couldn't get \"UUID\" tag for " << path_;
    throw BlockDeviceException("Unable to probe block device UUID");
  }
  std::string uuid_str(uuid);
  free(uuid);
  return uuid_str;
}

MountableBlockDevice::~MountableBlockDevice() {
  try {
    CloseMount();
    if (is_frozen_) {
      Thaw();
    }
  } catch (const std::runtime_error &e) {
    LOG(ERROR) << e.what();
  } catch (...) {
    LOG(ERROR) << "Caught non-exception type in destructor";
  }
}

} // datto_linux_client namespace
