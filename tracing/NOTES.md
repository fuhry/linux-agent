# block trace notes
This is a collection of scattered block trace data.

*THIS HAS NOT BEEN UPDATED SINCE MID 2013*

## Relay Channels
Block trace dumps information into userspace using relay channels.
https://www.kernel.org/doc/Documentation/filesystems/relay.txt

Each relay channel has a number of subbuffers, each a certain size.
When a subbuffer fills, poll() in userspace will return.

However, a trace is likely much smaller than a subbuffer, so poll
will not return until a reasonable amount of data has been written.

Having a fairly short polling timeout is then a good idea.

## pdu\_len
Some types of traces, (`BLK_TN_PROCESS`, `BLK_TN_TIMESTAMP`, `BLK_TN_MESSAGE` ?)
contain data after the trace itself. `pdu_len` specifies how long this extra data
is, and should be read from the trace.

## Per CPU
Each relay buffer is per CPU, so either one thread checking all file
descriptors or one per each file descriptor is needed.

There was some sort of CPU locking using `sched_setaffinity` that might
help improve performance (?). The /dropped file contains the number of
relay channel messages (i.e. `blk_io_traces`) which should be checked
periodically.

## Poor cleanup on process failure
*The below seems to be incorrect? Cleaning up after this type of failure still
proves to be a challenge*

In order to have a good state on process failure (i.e. a process that
fails to call BLKTRACESTOP and BLKTRACETEARDOWN on exit) it must be 
recleaned up. This can be done by calling BLKTRACESETUP, BLKTRACESTART,
BLKTRACESTOP, and BLKTRACETEARDOWN in that order, and then calling the
first two again.

