# Linux Agent
The Linux Agent is client side software that creates and transfers image-based
backups of the block devices on a Linux machine. 

## Table of Contents
* [Project Overview](#project-overview)
* [System Requirements](#system-requirements)
* [Build & Installation Instructions](#build--installation-instructions)
* [Usage](#usage)
* [How to Submit a Bug Report](#how-to-submit-a-bug-report)
* [How Does It Work?](#how-does-it-work)  

## Project Overview
The intention of this project is to create a general purpose tool to perform
image based backups of Linux machines.

## System Requirements
* Kernel version 2.6.32+
* Filesystem is ext4, ext3, ext2, or xfs

## Build & Installation Instructions
* All
    * Remember to `git submodule init && git submodule update` to get the
      submodule projects
* Debian-based (e.g. Ubuntu, Mint)
    * You can build the package from
      [linux-agent-deb](http://github-server.hq.datto.lan/ngarvey/linux-agent-deb),
      or just `./install_deps` to get the dependencies and `./one_step_build`
      to build it.
* Red-Hat based
    * Packages soon! But not yet. So follow the other directions for now.
* Other
    * You will need to go through the `install_deps` script and install the
      files needed using whatever your package manager is. Then, run
      `./one_step_build`. Let me know and I'll be happy to help with this.

## Usage
* Starting the daemon
    * If you installed from the package, it is probably already running.
      Otherwise, just type `dattod`.
    * If you built from source, do `./build/dattod`
* Requesting a backup
    1. Setup the destination server
        1. Make a sparse file to back the image. On the system to backup, run
           `blockdev --getsize64 /dev/YOUR_BD` to get the size in bytes of the
           block device to backup. Then, copy that number and do `truncate
           --size=SIZE_OF_BD block_dev.datto` on the destination system.
        2. Start nbd-server on the destination system. `nbd-server 12345
           $(readlink -f block_dev.datto)` to start it on port 12345
    2. Get the UUID of the block device to backup by running `blkid` and
       copying it out.
    3. Request a backup with `./build/dattocli startbackup --full --device-pair
       UUID_FROM_BLKID DESTINATION_HOST 12345`
     
## How to Submit a Bug Report
* Submit it using JIRA http://jira.hq.datto.lan/browse/LP
    * If you aren't sure if something is a bug, report it anyway! Rather have
      too many than too few.

## How Does It Work?
* http://ngarveysiris.hq.datto.lan/linux_agent.pdf
