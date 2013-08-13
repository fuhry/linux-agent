# Linux Agent
The Linux Agent is client and server side software that supports image based backups of the block devices on a Linux machine. 


## Project Overview
The intention of this project is to create a general purpose tool to perform image based backups of Linux machines. While we are looking to release a tool that functions under systems other than aurora-based Datto products (e.g. SIRIS, Alto), backing up to our products is still the focus. 

As this is a general purpose tool, we will be open sourcing the code base under GPLv2. This also allows us to leverage existing code under a GPL-compatible license.

### Technologies

* [blktrace](http://www.cse.unsw.edu.au/~aaronc/iosched/doc/blktrace.html)
* [device-mapper](http://www.sourceware.org/dm/)
* [Network Block Device](http://nbd.sourceforge.net/)
* filesystems
	* [ext2-4](http://en.wikipedia.org/wiki/Extended_file_system)
	* [XFS](http://en.wikipedia.org/wiki/XFS)
	* [Reiser](http://en.wikipedia.org/wiki/ReiserFS)

### Scope

* Supported configurations
	* Distributions
		* RHEL 5+
		* Ubuntu 12.04+
	* Architectures
		* x86 and x64
	* Kernel versions
		* 2.6.18+
	* They must be using LVM
		* Real requirement is device-mapper, but LVM uses it
	* Need to handle poor network connections

### Required components
* Executable that performs the backup
* Command line interface to communicate with the backup executable
* Client side HTTP server to allow the SIRIS to interface with the backup executable

## Directory Structure

This is not determined at this point. This will be filled out in the near future.

## Implementation Details

The goal is to [transfer](#transfer) a [consistent](#consistent) image, both [full](#full) and [incremental](#incremental), of the client block device in an [efficient](#efficient) manner.

### Transfer
[Network Block Device](http://nbd.sourceforge.net/) is the means of transport. This allows us to write each block to the location we want, and allows the NBD to be backed by a sparse file on the SIRIS. We need to [make sure to set the schedule to deadline](http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=447638) with NBD.

### Consistent
Consistency is achieved through device-mapper and blktrace.

#### device-mapper
device-mapper is crucial to making sure that the image we backup to the SIRIS is in a consistent state. device-mapper has a *snapshot* target that uses a Copy on Write method to make a copy of a block before it is overwritten.

A fundamental challenge of this project is where to put the copy of the original block.

There are three options:
* Hard drive
    * Pros:
        * Permanent storage
        * Plenty of room, don't to worry (as much) about running out of disk space
    * Cons:
        * Difficult, would need to resize the disk partitions or try to manipulate the filesystem
        * Will *really* [slow down I/O](http://www.mysqlperformanceblog.com/2013/07/09/lvm-read-performance-during-snapshots/)
* RAM
    * Pros:
        * Fast to read and write
        * Easy to implement
    * Cons:
        * Limited, it isn't unreasonable to encounter the situation where the user runs out of RAM 
* Network
    * Pros:
        * Permanent
        * Effectively unlimited storage
        * Easy to implement 
    * Cons:
        * Unreliable
        * Requires special handling on the SIRIS

Unfortunately, the cons for each method are blockers. As such, the only reasonable solution appears to be a hybrid approach. This hybrid is a buffered network snapshot.

##### Buffered network snapshot

In order to implement this, we need to create a dynamic virtual block device. The only apparent way ([see 43 "block device in userland"](https://www.kernel.org/doc/Documentation/devices.txt)) to do this is by running a NBD server locally and having that NBD server buffer on write and free that buffer on a read.

However we need to be careful, if the dynamic NBD block device is read by device-mapper (without being prompted by our code) then we will lose that data during the backup. device-mapper's behavior here is unknown, but the format of the snapshot [is known](https://people.gnome.org/~markmc/code/merge-dm-snapshot.c). As such, we can parse the snapshot *as it's being created* if needed.

This will work as long as we can send blocks over the network faster than they are being written, otherwise the local NBD server will eventually run out of RAM trying to buffer the writes. There is no solution to this other than failing early and often with a slow network. This shouldn't be a common issue. We are only sending the original data over the network, writing the same block multiple times won't cause the buffer to fill. 

#### blktrace

blktrace can be used in a situation where we can't use device-mapper. device-mapper is required for the Linux Agent. However, until recently, you couldn't install a /boot partition with device-mapper. We still need to backup /boot even if it isn't a device-mapper device in order to do any real work with the backed-up image.

To handle this case, we can just copy the entire /boot partition block by block, and then copy the blocks that have changed during the copy operation again. We can iterate this process until the number of changed blocks during the copy is 0.

This will work as long as we can send blocks over the network faster than they are being written, otherwise this process will never terminate. This is reasonable for a /boot partition.

### Full

Performing a full backup without bringing over unused blocks on the device requires filesystem parsing code specific to each filesystem.

If we can use device mapper, then a consistent image can be obtained and we can copy each in-use block from the source block device to the same location on the NBD.

If we can't use device mapper, parsing the filesystem is not an option as we have no easy way to determine if the metadata parsed is valid or not. As such, we need to copy all of the blocks in use and track the blocks modified during the copy and copy them after. See blktrace above.

### Incremental

Taking an incremental backup requires knowing which blocks have changed between now and the most recent backup. blktrace provides us just that.

Even though blktrace does a lot of the work for us, we need to be careful that we don't miss any writes during any stage of the backup process. In particular, it's hard to determine which backup a write belongs to if it occurs between the "device mapper block device freeze" request and the "blktrace data structure freeze" request. The safest answer to this question is 'both'. This behavior is achievable by creating a second, empty, blktrace data structure, and writing to both blktrace data structures during that period. 

### Efficient

Efficient means a couple things. First, we need to make sure the performance of the system doesn't degrade too much while the backup is occurring. Second, we need to keep in mind the amount of RAM usage during operation. Both tracing the blocks and the network buffer during backups could potentially take a very large amount of RAM if done poorly. Last, we need to clear out the blktrace buffer quickly to prevent dropped trace data. If we drop a single trace we have no way to know if we are missing something or not without parsing the entire device.
