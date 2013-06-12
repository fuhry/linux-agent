#!/usr/bin/env python

__author__ = 'ngarvey'

import ctypes
import ctypes.util
import dmconstants
import sys
import subprocess
import os


# libdevmapper setup
_libdevmapper_path = ctypes.util.find_library('devmapper')
_libdevmapper = ctypes.CDLL(_libdevmapper_path, use_errno=True)


# We need to be sure all kernel modules we might use are loaded.
# While shelling out is something we are trying to avoid, there
# don't appear to be better options (python-kmod appears to have
# too many dependencies)
for module_name in ["dm_snapshot"]:
    subprocess.check_call(['modprobe', module_name])


# Dummy class to help with type checking
class _DmTask(ctypes.Structure):
    pass


class _DmInfo(ctypes.Structure):
    _fields_ = [("exists", ctypes.c_int),
                ("suspended", ctypes.c_int),
                ("live_table", ctypes.c_int),
                ("inactive_table", ctypes.c_int),
                ("open_count", ctypes.c_int32),
                ("event_nr", ctypes.c_uint32),
                ("major", ctypes.c_uint32),
                ("minor", ctypes.c_uint32),
                ("read_only", ctypes.c_int),
                ("target_count", ctypes.c_int32)]


# According to lvm2-2.02.66/libdm/ioctl/libdm-iface.c:L1759, only
# DM_DEVICE_RESUME, DM_DEVICE_REMOVE, and DM_DEVICE_RENAME should run with a cookie
#
# However, CREATE calls RESUME (or, does a CREATE trigger a RESUME?),
# so all CREATES need a cookie too
class UDevCookie:
    """Synchronize libdevmapper udev operations

    UDevCookie should be instantiated and passed to certain devicemapper functions.
    It acts as a way for libdevmapper to synchronize with udev triggers so it doesn't attempt to
    create paths that udev has already created. In general, one cookie should be used per group of operations.

    UDevCookie is designed to be used with the Python "with" statement. For example:
    with devicemapper.UDevCookie() as cookie:
        devicemapper.do_some_operation(udev_cookie=cookie)
        devicemapper.do_some_other_operation(param1, param2, udev_cookie=cookie)

    At the end of the with block, the cookie will be cleaned up.
    """
    # This is used by ctypes when this object is passed to a function
    _as_parameter_ = None

    _udev_cookie = None

    def __init__(self):
        self._udev_cookie = ctypes.c_uint32()
        _libdevmapper.dm_udev_create_cookie(ctypes.byref(self._udev_cookie))
        self._as_parameter_ = ctypes.byref(self._udev_cookie)

    # http://docs.python.org/2/whatsnew/2.6.html#pep-343-the-with-statement
    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        _libdevmapper.dm_udev_wait(self._udev_cookie)


class _DmTarget(object):
    start = None
    size = None
    target_type = None
    params = None

    def __init__(self, start=None, size=None, target_type=None, params=None):
        self.start = int(start)
        self.size = int(size)
        self.target_type = target_type
        self.params = params


def _handle_err(result, func, args):
    errno = ctypes.get_errno()
    if errno:
        strerr = os.strerror(errno)
        raise Exception("{:s} ({:s}) - ERR{:d} {:s}".format(func.__name__, str(result), errno, strerr))
    return args


# Enable logging
_libdevmapper.dm_log_init_verbose.restype = None
_libdevmapper.dm_log_init_verbose.errcheck = _handle_err
if os.environ['DEBUG']:
    _libdevmapper.dm_log_init_verbose(1)

## This section initializes the parameters and return values for all of the
## _libdevmapper functions. This is used by ctypes to do type checking and
## conversion.

# dm_udev_create_cookie
_libdevmapper.dm_udev_create_cookie.argtypes = [ctypes.POINTER(ctypes.c_uint32)]
_libdevmapper.dm_udev_create_cookie.errcheck = _handle_err

# dm_udev_wait
_libdevmapper.dm_udev_wait.argtypes = [ctypes.c_uint32]
_libdevmapper.dm_udev_wait.errcheck = _handle_err

# dm_udev_complete
_libdevmapper.dm_udev_complete.argtypes = [ctypes.c_uint32]
_libdevmapper.dm_udev_complete.errcheck = _handle_err

# dm_task_set_cookie
_libdevmapper.dm_task_set_cookie.argtypes = [ctypes.POINTER(_DmTask), ctypes.POINTER(ctypes.c_uint32), ctypes.c_uint16]
_libdevmapper.dm_task_set_cookie.errcheck = _handle_err

# dm_task_get_info
_libdevmapper.dm_task_get_info.argtypes = [ctypes.POINTER(_DmTask), ctypes.POINTER(_DmInfo)]
_libdevmapper.dm_task_get_info.errcheck = _handle_err

# dm_task_create
_libdevmapper.dm_task_create.restype = ctypes.POINTER(_DmTask)
_libdevmapper.dm_task_create.errcheck = _handle_err

# dm_task_set_uuid
_libdevmapper.dm_task_set_uuid.argtypes = [ctypes.POINTER(_DmTask), ctypes.c_char_p]
_libdevmapper.dm_task_set_uuid.errcheck = _handle_err

# dm_task_run
_libdevmapper.dm_task_run.argtypes = [ctypes.POINTER(_DmTask)]
_libdevmapper.dm_task_run.errcheck = _handle_err

# dm_get_next_target
_libdevmapper.dm_get_next_target.argtypes = [ctypes.POINTER(_DmTask), ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint64),
                                             ctypes.POINTER(ctypes.c_uint64), ctypes.POINTER(ctypes.c_char_p),
                                             ctypes.POINTER(ctypes.c_char_p)]
_libdevmapper.dm_get_next_target.restype = ctypes.c_void_p
_libdevmapper.dm_get_next_target.errcheck = _handle_err

# dm_task_add_target
_libdevmapper.dm_task_add_target.argtypes = [ctypes.POINTER(_DmTask), ctypes.c_uint64, ctypes.c_uint64, ctypes.c_char_p,
                                             ctypes.c_char_p]
_libdevmapper.dm_task_add_target.errcheck = _handle_err

# dm_task_set_name
_libdevmapper.dm_task_set_name.argtypes = [ctypes.POINTER(_DmTask), ctypes.c_char_p]
_libdevmapper.dm_task_set_name.errcheck = _handle_err


def _get_targets(dm_table_task):
    targets = []

    next = None
    start = ctypes.c_uint64(-1)
    size = ctypes.c_uint64(-1)
    target_type = ctypes.c_char_p()
    params = ctypes.c_char_p()
    next = _libdevmapper.dm_get_next_target(dm_table_task, ctypes.c_void_p(next), ctypes.byref(start),
                                            ctypes.byref(size), ctypes.byref(target_type), ctypes.byref(params))

    targets.append(_DmTarget(start.value, size.value, target_type.value, params.value))
    while next:
        next = _libdevmapper.dm_get_next_target(dm_table_task, ctypes.c_void_p(next), ctypes.ctypes.byref(start),
                                                ctypes.byref(size), ctypes.byref(target_type), ctypes.byref(params))
        targets.append(_DmTarget(start.value, size.value, target_type.value, params.value))

    return targets


def _get_complete_table_task(source_name):
    dm_table_task = _libdevmapper.dm_task_create(dmconstants.DM_DEVICE_TABLE)
    _libdevmapper.dm_task_set_name(dm_table_task, source_name)
    _libdevmapper.dm_task_run(dm_table_task)
    return dm_table_task


def get_num_sectors(source_name):
    """ Return the number of sectors of the devicemapper device source_name"""
    dm_table_task = _get_complete_table_task(source_name)
    targets = _get_targets(dm_table_task)

    device_size_sectors = sum([target.size for target in targets])
    return device_size_sectors


def duplicate_table(source_name, destination_name=None, udev_cookie=None):
    """ Duplicate a devicemapper table

    The devicemapper table referred to by source_name will be copied to destination_name. A UDevCookie class should be
    given as the udev_cookie argument.
    """
    if not destination_name:
        destination_name = source_name + "_dup"

    # Get executed TABLE task
    dm_table_task = _get_complete_table_task(source_name)

    # Create CREATE task
    dm_create_task = _libdevmapper.dm_task_create(dmconstants.DM_DEVICE_CREATE)
    _libdevmapper.dm_task_set_name(dm_create_task, destination_name)

    # Copy targets from TABLE task to CREATE task
    targets = _get_targets(dm_table_task)
    for target in targets:
        _libdevmapper.dm_task_add_target(dm_create_task, target.start, target.size, target.target_type, target.params)

    if udev_cookie:
        _libdevmapper.dm_task_set_cookie(dm_create_task, udev_cookie, 0)

    # Execute CREATE
    _libdevmapper.dm_task_run(dm_create_task)

    return destination_name


def create_snapshot(source_name, cow_file_path, snapshot_name=None, chuck_size=32, udev_cookie=None):
    """ Create a devicemapper snapshot of a devicemapper block device

    A snapshot of source_name will be created. The cow_file_path should be the name of the block device that
    will contain the copy-on-write data used by the devicemapper snapshot.

    See https://www.kernel.org/doc/Documentation/device-mapper/snapshot.txt for devicemapper snapshot information.

    Keyword arguments:
    snapshot_name -- The name given to the devicemapper block device. Defaults to source_name + '_cow'
    chuck_size -- The COW file tracks changes in 'chunks'. chunk_size is the number of sectors for that chunk.
    udev_cookie -- The UDevCookie instance that should be used for this operation
    """
    if not snapshot_name:
        snapshot_name = source_name + "_cow"

    # Create CREATE task
    dm_create_task = _libdevmapper.dm_task_create(dmconstants.DM_DEVICE_CREATE)
    _libdevmapper.dm_task_set_name(dm_create_task, snapshot_name)

    # Set up snapshot parameters
    source_num_sectors = get_num_sectors(source_name)
    source_info = get_info(source_name)

    params = "{:d}:{:d} {:s} {:s} {:d}".format(source_info.major, source_info.minor, cow_file_path, 'p', chuck_size)
    _libdevmapper.dm_task_add_target(dm_create_task, 0, source_num_sectors, "snapshot", params)

    if udev_cookie:
        _libdevmapper.dm_task_set_cookie(dm_create_task, udev_cookie, 0)

    # Execute CREATE
    _libdevmapper.dm_task_run(dm_create_task)

    return snapshot_name


def get_info(source_name):
    """ Returns a devicemapper info object for a given source_name.

    This is similiar to the shell cmd: 'dmsetup info source_name'
    """
    dm_info_task = _libdevmapper.dm_task_create(dmconstants.DM_DEVICE_INFO)
    _libdevmapper.dm_task_set_name(dm_info_task, source_name)

    _libdevmapper.dm_task_run(dm_info_task)

    dm_info = _DmInfo()
    _libdevmapper.dm_task_get_info(dm_info_task, dm_info)

    return dm_info


def create_snapshot_origin(source_name, snapshot_origin_name=None, udev_cookie=None):
    """ Create a devicemapper snapshot-origin of a devicemapper block device

    A snapshot-origin of source_name will be created. source_name should be a devicemapper device that has
    COW file backed snapshots. This means writes to the snapshot-origin will update the COW file with
    the original blocks the device contained - meaning the snapshot will be kept consistent.

    See https://www.kernel.org/doc/Documentation/device-mapper/snapshot.txt for devicemapper snapshot information.

    Keyword arguments:
    snapshot_origin_name -- The name given to the devicemapper block device. Defaults to source_name + '_orig'
    udev_cookie -- The UDevCookie instance that should be used for this operation
    """
    if not snapshot_origin_name:
        snapshot_origin_name = source_name + "_orig"

    # Create CREATE task
    dm_create_task = _libdevmapper.dm_task_create(dmconstants.DM_DEVICE_CREATE)
    _libdevmapper.dm_task_set_name(dm_create_task, snapshot_origin_name)

    # Set up snapshot-origin parameters
    source_info = get_info(source_name)
    params = "{:d}:{:d}".format(source_info.major, source_info.minor)

    source_num_sectors = get_num_sectors(source_name)

    # Add snapshot-origin target
    _libdevmapper.dm_task_add_target(dm_create_task, 0, source_num_sectors, "snapshot-origin", params)

    if udev_cookie:
        _libdevmapper.dm_task_set_cookie(dm_create_task, udev_cookie, 0)

    # Execute CREATE
    _libdevmapper.dm_task_run(dm_create_task)

    return snapshot_origin_name


def load_from_dm_device(source_name, dest_name):
    """ Loads a devicemapper table stored at source_name into dest_name.

    Note that this does not refresh the dest_name, so a resume will need to be done after this command for the new
    table to take effect.
    """
    dm_table_task = _get_complete_table_task(source_name)

    dm_load_task = _libdevmapper.dm_task_create(dmconstants.DM_DEVICE_RELOAD)
    _libdevmapper.dm_task_set_name(dm_load_task, dest_name)

    # Copy targets from TABLE task to RELOAD task
    targets = _get_targets(dm_table_task)
    for target in targets:
        _libdevmapper.dm_task_add_target(dm_load_task, target.start, target.size, target.target_type, target.params)

    # Execute RELOAD
    _libdevmapper.dm_task_run(dm_load_task)


def suspend(source_name):
    """ Suspend the devicemapper device source_name. I/O will blocked until the device is resumed (!!) """
    dm_suspend_task = _libdevmapper.dm_task_create(dmconstants.DM_DEVICE_SUSPEND)
    _libdevmapper.dm_task_set_name(dm_suspend_task, source_name)

    # Execute SUSPEND
    _libdevmapper.dm_task_run(dm_suspend_task)


def resume(source_name, udev_cookie=None):
    """ Resume the suspended devicemapper device source_name.

    This will re-enable all I/O to a device and make the table in the 'INACTIVE' slot 'LIVE'. See man 8 dmsetup.

    Keyword arguments:
    udev_cookie -- The UDevCookie instance that should be used for this operation
    """
    dm_resume_task = _libdevmapper.dm_task_create(dmconstants.DM_DEVICE_RESUME)
    _libdevmapper.dm_task_set_name(dm_resume_task, source_name)

    # Execute RESUME
    if udev_cookie:
        _libdevmapper.dm_task_set_cookie(dm_resume_task, udev_cookie, 0)

    _libdevmapper.dm_task_run(dm_resume_task)


if __name__ == "__main__":
    source_name = sys.argv[1]
    loop_dev = sys.argv[2]

    with UDevCookie() as cookie:
        dup_name = duplicate_table(source_name, udev_cookie=cookie)

        suspend(source_name)
        try:
            create_snapshot(dup_name, loop_dev, udev_cookie=cookie)
            snapshot_origin_name = create_snapshot_origin(dup_name, udev_cookie=cookie)
            load_from_dm_device(snapshot_origin_name, source_name)
        finally:
            resume(source_name, udev_cookie=cookie)