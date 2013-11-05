# Linux Agent
The Linux Agent is client side software that creates and transfers image-based
backups of the block devices on a Linux machine. 

## Project Overview
The intention of this project is to create a general purpose tool to perform
image based backups of Linux machines. However, backing up to our products is
still the focus.  As this is a general purpose tool, we will be open sourcing
the code base under GPLv2. This also allows us to leverage existing code under
a GPL-compatible license.

### Technologies

* [blktrace](http://www.cse.unsw.edu.au/~aaronc/iosched/doc/blktrace.html)
* [Network Block Device](http://nbd.sourceforge.net/)
* filesystems
	* [ext2-4](http://en.wikipedia.org/wiki/Extended_file_system)
	* [XFS](http://en.wikipedia.org/wiki/XFS)
	* [Reiser](http://en.wikipedia.org/wiki/ReiserFS)

### Scope

* Supported configurations
	* Distributions
		* RHEL 5+ (?)
		* Ubuntu 12.04+
	* Architectures
		* x86 and x64
	* Kernel versions
		* 2.6.18+ (?)
	* Need to handle poor network connections

### Required components
* Executable that performs the backup
* Command line interface to communicate with the backup executable
* Client side HTTP server to allow the SIRIS to interface with the backup
  executable

## Directory Structure

* `client/`: Code to run on the client computer
    * `build/`: Directory created during the build process
    * `cmake/`: Code for the build process
    * `dattocli/`: Command line tool
    * `dattod/`: Contains dattod.cc which has the main() function
    * `dependency_files/`: Text files etc. needed during execution
    * `docs/`: Developer client documentation
    * `messages/`: .proto files used in interprocess communication
    * `test/`: Unit tests
    * `CMakeLists.txt`: Build file used by CMake
    * `install_deps`: Script which installs the dependancies to begin working
    * `one_step_build`: Script which builds a binary
    * `run_tests`: Script which runs the unit tests
    * `until_failure`: Script which runs the unit tests until one fails
* `helper_scripts/`: Scattered files used to automate some process
* `proof_of_concept/`: Legacy files which contain some informational value

## Implementation Details

The goal is to [transfer](#transfer) a [consistent](#consistent) image, both
[full](#full), [diff](#diff) and [incremental](#incremental), of the client
block device in an [efficient](#efficient) manner.

### Transfer

[Network Block Device](http://nbd.sourceforge.net/) is the means of transport.
This allows us to write each block to the location we want, and allows the NBD
to be backed by a sparse file on the SIRIS. We need to [make sure to set the
schedule to deadline](http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=447638)
with NBD.

### Consistent
Consistency is achieved through the FIFREEZE ioctl and blktrace.

### FIFREEZE ioctl

The [FIFREEZE ioctl](http://goo.gl/4Qmdqg) will both make the file system
consistent on disk and suspend all writes. While extremely useful for our
application, we need to use this with caution. Freezing the filesystem for too
long will certainly cause instability with many processes.

#### blktrace
blktrace allows us to track the changes that occur both during the backup and
in-between backups (incrementals). By tracking writes, we can iteratively
backup blocks that have changed until we have managed to sync over all blocks
that have changed on the client.

### Full

Performing a full backup without bringing over unused blocks on the device
requires filesystem parsing code specific to each filesystem. To determine the
in-use blocks on a filesystem, we need to read the metadata for that
filesystem. However, as the disk is changing as we are taking the backup, we
need to freeze the filesystem while reading the metadata off the disk. Once the
metadata is read, we can perform the synchronization.


### Diff

Diffs are a work in progress, but the big idea is that they will be just like a
full, but only the different in-use blocks will be marked as "unsynced".

### Incremental

Taking an incremental backup requires knowing which blocks have changed between
now and the most recent backup. blktrace provides us just that.

Even though blktrace does a lot of the work for us, we need to be careful that
we don't miss any writes during any stage of the backup process. This means we
need to flush the following buffers (in this order!):

* Filesystem buffers (FIFREEZE does this)
* Block device buffers (BLKFLSBUF ioctl does this)
* Block trace buffers (CpuTracer::FlushBuffer does this)

### Efficient

Efficient means a couple things. First, we need to make sure the performance of
the system doesn't degrade too much while the backup is occurring. Second, we
need to keep in mind the amount of RAM usage during operation. The highest risk
of running out of memory will be during a diff backup, as both tracking the
changes sectors and holding the base image in memory will require a lot of
memory. It might be worth creating a separate thread that will cancel the
process if memory pressure gets too high, but for now we don't worry about it.
