__author__ = 'ngarvey'

import sys

# Seek values taken from magic/Magdir/filesystems in 'file' source
def get_filesystem(file_obj):
    pass


def is_ext(file_obj):
    file_obj.seek(0x438)
    file_obj.seek(0x438)
    is_ext = [ord(char) for char in file_obj.read(2)] == [0x53, 0xEF]
    file_obj.seek(0)
    return is_ext


def is_ext2(file_obj):
    if not is_ext(file_obj):
        return False

    file_obj.seek(0x45c)
    # This checks for a journal, which ext2 doesn't have
    is_ext2 = not ord(file_obj.read(1)) & 0x4
    file_obj.seek(0)
    return is_ext2


def is_ext3(file_obj):
    if not is_ext(file_obj):
        return False

    file_obj.seek(0x45c)

    if is_ext2(file_obj):
        return False

    file_obj.seek(0x460)
    incompat = file_obj.read(1)

    if incompat > 0x3F:
        # This is ext4
        return False
    else:
        file_obj.seek(0x464)
        ro_compat = file_obj.read(1)
        file_obj.seek(0)
        return ro_compat < 0x08


def is_ext4(file_obj):
    return is_ext(file_obj) and not is_ext2(file_obj) and not is_ext3(file_obj)


def is_xfs(file_obj):
    file_obj.seek(0)
    return [ord(char) for char in file_obj.read(4)] == [0x58, 0x46, 0x53, 0x42]


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print "usage:", sys.argv[0], "<block_dev>"
        exit(1)
    with open(sys.argv[1]) as block_dev:
        print "ext", is_ext(block_dev)
        print "ext2", is_ext2(block_dev)
        print "ext3", is_ext3(block_dev)
        print "ext4", is_ext4(block_dev)
        print "xfs", is_xfs(block_dev)