# Linux Agent
The Linux Agent is client and server side software that supports image based backups of the block devices on a Linux machine. 


## Project Overview
The intention of this project is to create a general purpose tool to perform image based backups of Linux machines. While we are looking to release a tool that functions under systems other than aurora-based Datto products (e.g. SIRIS, Alto), backing up to our products is still the focus. 

As this is a general purpose tool, we will be open sourcing the code base under GPLv2. This also allows us to leverage existing code under a GPL-compatible license.

### Technologies

* [blktrace](http://www.cse.unsw.edu.au/~aaronc/iosched/doc/blktrace.html)
* [device-mapper](http://www.sourceware.org/dm/)
* file systems
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
		* This is needed so we know they have device-mapper
		* Things like their /boot drive we will still backup
	* Need an average speed LAN connection

### Required components
* Binary that performs the backup
* Command line interface to communicate with the backup binary
* Client side HTTP server to allow the SIRIS to interface with the backup binary

### Required components on the SIRIS (not this repo) 
* Code to act on the image, namely performing virtualization and restore capabilities
* A binary to perform a diff backup
	* That is, calculate the differences between the current drive and the server side image and only copy the differences between the two.
