#include "block_device/mountable_block_device.h"
#include "block_device/block_device_exception.h"
#include <fstream>
#include <glog/logging.h>
#include <linux/limits.h>
#include <map>
#include <string>

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
    std::string path = line_str.substr(0, first_space - 1);
    std::string mount_point =
        line_str.substr(first_space + 1, second_space - 1);

    std::string real_path;
    char real_path_buf[PATH_MAX];
    if (readlink(path.c_str(), real_path_buf, PATH_MAX) == -1) {
      int error = errno;
      if (error == ENOENT) {
        continue;
      } else if (error == EACCES) {
        continue;
      } else if (error == ENOTDIR) {
        continue;
      } else if (error == EINVAL) {
        real_path = path;
      } else {
        PLOG(ERROR) << "Error during readlink";
        throw datto_linux_client::BlockDeviceException("readlink");
      }
    } else {
      real_path = std::string(real_path_buf);
    }

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

    mounted_devices[real_path] = mount_point;
  }

  return mounted_devices;
}
}

namespace datto_linux_client {

MountableBlockDevice::MountableBlockDevice(std::string block_path)
    : BlockDevice(block_path) { }

bool MountableBlockDevice::IsMounted() {
  // TODO These checks should be based on major/minor number, not on path.
  // mknod can be used to create "block special" type devices all over the
  // file system that all refer to the same block device but don't
  // show up as symbolic links
  auto mounted_devices = GetMountedDevices();
  return mounted_devices.find(block_path_) != mounted_devices.end();
}

} // datto_linux_client
