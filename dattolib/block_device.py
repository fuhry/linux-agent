import ctypes
import ctypes.util
from fcntl import ioctl
import struct
import os
import os.path


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


# libc setup and related functions

class MntEnt(ctypes.Structure):
    """ Struct defined in mntent.h """
    _fields_ = [
        ("mnt_fsname", ctypes.c_char_p),
        ("mnt_dir", ctypes.c_char_p),
        ("mnt_type", ctypes.c_char_p),
        ("mnt_opts", ctypes.c_char_p),
        ("mnt_freq", ctypes.c_int),
        ("mnt_passno", ctypes.c_int)
    ]


_libc_path = ctypes.util.find_library('c')
if not _libc_path:
    raise EnvironmentError("libc library not found (!!)")
_libc = ctypes.CDLL(_libc_path, use_errno=True)
_libc.getmntent.restype = ctypes.POINTER(MntEnt)


def get_mount_point(path):
    with open("/proc/mounts") as mounts:
        # See "man 2 mntent" to understand this section
        # and the variable names
        file_p = _libc.fdopen(mounts.fileno(), "r")
        while True:
            # This is a struct containing pointers to static memory.
            # As such, we shouldn't copy the mntent structure as the values
            # it points to will be overwitten in the getmntent call
            mntent = _libc.getmntent(file_p)
            if not mntent:
                break

            # Make sure the device starts with / or we know it's some
            # virtual thing we don't want to look at
            if mntent.contents.mnt_fsname[0] != '/':
                continue
            fsname = mntent.contents.mnt_fsname

            if os.path.islink(fsname):
                fsname_dir = os.path.dirname(fsname)
                fsname = os.path.join(fsname_dir, os.readlink(fsname))
            if os.path.abspath(path) == os.path.abspath(fsname):
                return mntent.contents.mnt_dir
    return None


if __name__ == "__main__":
    import sys
    path = sys.argv[1] if len(sys.argv) > 1 else "/dev/sda1"
    print "fs:", get_fs(path)
    print "uuid:", get_uuid(path)
    print "size:", get_size(path)
    print "block size:", get_block_size(path)
    print "mount point:", get_mount_point(path)
