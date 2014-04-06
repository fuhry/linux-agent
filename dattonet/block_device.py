import ctypes
import ctypes.util
from fcntl import ioctl
import struct


# blkid setup and related functions
_blkid_path = ctypes.util.find_library('blkid')
if not _blkid_path:
    raise EnvironmentError("blkid library not found")

_blkid = ctypes.CDLL(_blkid_path, use_errno=True)
_blkid.blkid_get_tag_value.restype = ctypes.c_char_p


def get_fs(path):
    """ Return filesystem on path"""
    return _blkid.blkid_get_tag_value(None, "TYPE", path)


def get_uuid(path):
    """ Return uuid of path"""
    return _blkid.blkid_get_tag_value(None, "UUID", path)


# ioctl setup and related functions

# These values were obtained with the following C program
#
# #include "linux/fs.h"
# #include <stdio.h>
#
# int main() {
#         printf("0x%lx\n", BLKGETSIZE64);
#         printf("0x%lx\n", BLKBSZGET);
# }
BLKGETSIZE64 = 0x80081272
BLKBSZGET = 0x80081270


def get_size(path):
    """ Return size in bytes of path"""
    with open(path) as device:
        buf = '\0' * 8
        buf = ioctl(device.fileno(), BLKGETSIZE64, buf)
        return struct.unpack('L', buf)[0]


def get_block_size(path):
    """ Return block size in bytes of path"""
    with open(path) as device:
        buf = '\0' * 8
        buf = ioctl(device.fileno(), BLKBSZGET, buf)
        return struct.unpack('L', buf)[0]


if __name__ == "__main__":
    import sys
    path = sys.argv[1] if len(sys.argv) > 1 else "/dev/sda1"
    print "fs:", get_fs(path)
    print "uuid:", get_uuid(path)
    print "size:", get_size(path)
    print "block size:", get_block_size(path)
