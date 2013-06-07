#!/usr/bin/env python

__author__ = 'ngarvey'

import ctypes
import ctypes.util
import dmconstants
import sys

# libdevmapper setup
_libdevmapper_path = ctypes.util.find_library('devmapper')
_libdevmapper = ctypes.CDLL(_libdevmapper_path, use_errno=True)

_libdevmapper.dm_log_init_verbose.restype = None
_libdevmapper.dm_log_init_verbose(10)


# Dummy class to help with type checking
class _DmTask(ctypes.Structure):
    pass


class DmTarget(object):
    start = None
    size = None
    target_type = None
    params = None

    def __init__(self, start=None, size=None, target_type=None, params=None):
        self.start = start
        self.size = size
        self.target_type = target_type
        self.params = params


## This section initializes the parameters and return values for all of the
## _libdevmapper functions. This is used by ctypes to do type checking and
## conversion.

# dm_task_create
_libdevmapper.dm_task_create.restype = ctypes.POINTER(_DmTask)

# dm_task_set_uuid
_libdevmapper.dm_task_set_uuid.argtypes = [ctypes.POINTER(_DmTask), ctypes.c_char_p]

# dm_task_run
_libdevmapper.dm_task_run.argtypes = [ctypes.POINTER(_DmTask)]

# dm_get_next_target
_libdevmapper.dm_get_next_target.argtypes = [ctypes.POINTER(_DmTask), ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint64),
                                             ctypes.POINTER(ctypes.c_uint64), ctypes.POINTER(ctypes.c_char_p),
                                             ctypes.POINTER(ctypes.c_char_p)]
_libdevmapper.dm_get_next_target.restype = ctypes.c_void_p

# dm_task_add_target
_libdevmapper.dm_task_add_target.argtypes = [ctypes.POINTER(_DmTask), ctypes.c_uint64, ctypes.c_uint64, ctypes.c_char_p,
                                             ctypes.c_char_p]

# dm_task_set_name
_libdevmapper.dm_task_set_name.argtypes = [ctypes.POINTER(_DmTask), ctypes.c_char_p]


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


def _copy_targets(dm_table_task, dm_create_task):
    targets = _get_targets(dm_table_task)
    for target in targets:
        _libdevmapper.dm_task_add_target(dm_create_task, target.start, target.size, target.target_type, target.params)


def duplicate_table(source_name, destination_name):
    # Create TABLE task
    dm_table_task = _libdevmapper.dm_task_create(dmconstants.DM_DEVICE_TABLE)
    _libdevmapper.dm_task_set_name(dm_table_task, source_name)

    # Create CREATE task
    dm_create_task = _libdevmapper.dm_task_create(dmconstants.DM_DEVICE_CREATE)
    _libdevmapper.dm_task_set_name(dm_create_task, destination_name)

    # Execute TABLE task
    _libdevmapper.dm_task_run(dm_table_task)

    # Copy targets from TABLE task to CREATE task
    _copy_targets(dm_table_task, dm_create_task)

    # Execute CREATE task
    _libdevmapper.dm_task_run(dm_create_task)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.stderr.write("usage: " + sys.argv[0] + " dm_source dm_destination\n")
        exit(1)
    duplicate_table(sys.argv[1], sys.argv[2])