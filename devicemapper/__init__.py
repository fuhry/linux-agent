#!/usr/bin/env python

__author__ = 'ngarvey'

import ctypes
import ctypes.util
import dmconstants
import sys
import os


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


class DmTarget(object):
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

# libudev setup
_libudev_path = ctypes.util.find_library('udev')
_libudev = ctypes.CDLL(_libudev_path, use_errno=True)


## This section initializes the parameters and return values for all of the
## _libdevmapper functions. This is used by ctypes to do type checking and
## conversion.

# libdevmapper setup
_libdevmapper_path = ctypes.util.find_library('devmapper')
_libdevmapper = ctypes.CDLL(_libdevmapper_path, use_errno=True)

# Enable logging
_libdevmapper.dm_log_init_verbose.restype = None
_libdevmapper.dm_log_init_verbose.errcheck = _handle_err
_libdevmapper.dm_log_init_verbose(1)

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


udev_cookie = ctypes.c_uint32(0)
_libdevmapper.dm_udev_create_cookie(ctypes.byref(udev_cookie))


# According to lvm2-2.02.66/libdm/ioctl/libdm-iface.c:L1759, only
# DM_DEVICE_RESUME, DM_DEVICE_REMOVE, and DM_DEVICE_RENAME should run with a cookie
#
# However, CREATE calls RESUME, so all CREATES need a cookie too
def _run_with_cookie(dm_task):
    global udev_cookie
    sys.stderr.write("Setting cookie\n")
    _libdevmapper.dm_task_set_cookie(dm_task, ctypes.byref(udev_cookie), 0)
    sys.stderr.write("Running task\n")
    _libdevmapper.dm_task_run(dm_task)


def _get_targets(dm_table_task):
    targets = []

    next = None
    start = ctypes.c_uint64(-1)
    size = ctypes.c_uint64(-1)
    target_type = ctypes.c_char_p()
    params = ctypes.c_char_p()
    next = _libdevmapper.dm_get_next_target(dm_table_task, ctypes.c_void_p(next), ctypes.byref(start),
                                            ctypes.byref(size), ctypes.byref(target_type), ctypes.byref(params))

    targets.append(DmTarget(start.value, size.value, target_type.value, params.value))
    while next:
        next = _libdevmapper.dm_get_next_target(dm_table_task, ctypes.c_void_p(next), ctypes.ctypes.byref(start),
                                                ctypes.byref(size), ctypes.byref(target_type), ctypes.byref(params))
        targets.append(DmTarget(start.value, size.value, target_type.value, params.value))

    return targets


def _get_complete_table_task(source_name):
    dm_table_task = _libdevmapper.dm_task_create(dmconstants.DM_DEVICE_TABLE)
    _libdevmapper.dm_task_set_name(dm_table_task, source_name)
    _libdevmapper.dm_task_run(dm_table_task)
    return dm_table_task


def get_num_sectors(source_name):
    dm_table_task = _get_complete_table_task(source_name)
    targets = _get_targets(dm_table_task)

    device_size_sectors = sum([target.size for target in targets])
    return device_size_sectors


def duplicate_table(source_name, destination_name=None):
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

    # Execute CREATE
    _run_with_cookie(dm_create_task)

    return destination_name


def create_snapshot(source_name, cow_file_path, snapshot_name=None, chuck_size=32):
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

    # Execute CREATE
    _run_with_cookie(dm_create_task)

    return snapshot_name


def get_info(source_name):
    dm_info_task = _libdevmapper.dm_task_create(dmconstants.DM_DEVICE_INFO)
    _libdevmapper.dm_task_set_name(dm_info_task, source_name)

    _libdevmapper.dm_task_run(dm_info_task)

    dm_info = _DmInfo()
    _libdevmapper.dm_task_get_info(dm_info_task, dm_info)

    return dm_info


def create_snapshot_origin(source_name, snapshot_origin_name=None):
    if not snapshot_origin_name:
        snapshot_origin_name = source_name + "_orig"

    # Create CREATE task
    dm_create_task = _libdevmapper.dm_task_create(dmconstants.DM_DEVICE_CREATE)
    _libdevmapper.dm_task_set_name(dm_create_task, snapshot_origin_name)

    # Set up snapshot parameters
    source_num_sectors = get_num_sectors(source_name)

    source_info = get_info(source_name)

    params = "{:d}:{:d}".format(source_info.major, source_info.minor)
    _libdevmapper.dm_task_add_target(dm_create_task, 0, source_num_sectors, "snapshot-origin", params)

    # Execute CREATE
    _run_with_cookie(dm_create_task)

    return snapshot_origin_name


def load_from_dm_device(source_name, dest_name):
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
    dm_suspend_task = _libdevmapper.dm_task_create(dmconstants.DM_DEVICE_SUSPEND)
    _libdevmapper.dm_task_set_name(dm_suspend_task, source_name)

    # Execute SUSPEND
    _libdevmapper.dm_task_run(dm_suspend_task)


def resume(source_name):
    dm_resume_task = _libdevmapper.dm_task_create(dmconstants.DM_DEVICE_RESUME)
    _libdevmapper.dm_task_set_name(dm_resume_task, source_name)

    # Execute RESUME
    _run_with_cookie(dm_resume_task)

if __name__ == "__main__":
    source_name = sys.argv[1]
    loop_dev = sys.argv[2]

    dup_name = duplicate_table(source_name)
    suspend(source_name)

    try:
        create_snapshot(dup_name, loop_dev)
        snapshot_origin_name = create_snapshot_origin(dup_name)
        load_from_dm_device(snapshot_origin_name, source_name)
    finally:
        resume(source_name)

    sys.stderr.write("Waiting on cookie\n")

    _libdevmapper.dm_udev_wait(udev_cookie)
