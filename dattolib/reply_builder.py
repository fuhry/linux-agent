import csv
import os
import re
import subprocess
from multiprocessing import cpu_count
from dattolib.block_device import \
    get_size, get_block_size, get_fs, \
    get_mount_point, get_used_bytes
from dattolib.reply_pb2 import Reply

# TODO Set in configure?
PID_FILE = "/var/run/dattod.pid"


def run_command(command):
    p = subprocess.Popen(command, stdout=subprocess.PIPE)
    (stdout, stderr) = p.communicate()
    return stdout.strip()


def get_lsb_release():
    ret_dict = {}
    try:
        import lsb_release

        ret_dict = lsb_release.get_distro_information()
    except ImportError:
        # Couldn't import lsb_release so try the lsb-release file
        try:
            with open("/etc/lsb-release") as lsb_file:
                lsb_reader = csv.reader(lsb_file, delimiter='=', quotechar='"')
                for row in lsb_reader:
                    key = row[0].lstrip('DISTRIB_')
                    ret_dict[key] = row[1]
        except IOError:
            # Couldn't read /etc/lsb-release so try lsb_release -a
            str_map = {'Distributor ID': 'ID',
                       'Description': 'DESCRIPTION',
                       'Release': 'RELEASE',
                       'Codename': 'CODENAME'}
            output = run_command(['lsb_release', '-a'])
            for line in output.split('\n'):
                match = re.match('^([^:]*):\t*(.*)$', line)
                if match and match.group(1) in str_map:
                    key = str_map[match.group(1)]
                    ret_dict[key] = match.group(2)
    if not ret_dict:
        ret_dict = {'error': "Couldn't get LSB information"}
    return ret_dict


def get_basic_info():
    reply = Reply()
    reply.type = Reply.BASIC_INFO
    # TODO version
    reply.basic_info_reply.agent_version = "v2.0"
    uname = os.uname()
    reply.basic_info_reply.hostname = uname[1]
    reply.basic_info_reply.uname_a = uname[2]
    reply.basic_info_reply.lsb_release_a = str(get_lsb_release())

    return reply


def set_block_devices(reply):
    UUID_DIR = "/dev/disk/by-uuid/"
    for filename in os.listdir(UUID_DIR):
        device = reply.complete_info_reply.block_devices.add()
        device.uuid = filename

        real_relative_path = os.readlink(UUID_DIR + filename)
        device.real_path = \
            os.path.abspath(os.path.join(UUID_DIR, real_relative_path))

        device.device_size = get_size(device.real_path)
        device.block_size = get_block_size(device.real_path)
        device.file_system = get_fs(device.real_path)
        mount_point = get_mount_point(device.real_path)
        if mount_point:
            device.mount_location = mount_point
            device.used_space = get_used_bytes(device.mount_location)


def get_complete_info():
    reply = Reply()
    reply.type = Reply.COMPLETE_INFO
    # TODO version
    reply.complete_info_reply.agent_version = "2.0"
    uname = os.uname()
    reply.complete_info_reply.hostname = uname[1]
    reply.complete_info_reply.kernel = uname[2]
    reply.complete_info_reply.os_name = uname[3]
    reply.complete_info_reply.arch = uname[4]
    reply.complete_info_reply.cpus = cpu_count()
    reply.complete_info_reply.lsb_release_a = str(get_lsb_release())

    with open("/proc/meminfo") as meminfo:
        total_ram = int(re.sub('[^0-9]', '', meminfo.readline())) * 1024
        reply.complete_info_reply.memory = str(total_ram)

    try:
        with open(PID_FILE) as datto_pid_file:
            dattod_pid = (datto_pid_file.read()).strip()

        with open("/proc/" + dattod_pid + "/stat") as dattod_stat:
            dattod_jiffs = int(dattod_stat.read().split()[21])

        jiffs_per_sec = int(run_command(["getconf", "CLK_TCK"]))

        # This is uptime from when the system booted
        dattod_uptime = dattod_jiffs / jiffs_per_sec

        with open("/proc/uptime") as uptime_file:
            system_uptime = int(float(uptime_file.read().split()[0]))

        reply.complete_info_reply.dattod_uptime_seconds = \
            system_uptime - dattod_uptime

    except IOError as e:
        # Just don't set it if we can't get it
        pass

    set_block_devices(reply)
    return reply
