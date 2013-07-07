#!/usr/bin/env python

# Helper script to test sending messages to the datto daemon (dattod)

import posix_ipc

QUEUE_NAME = "/dattod"

queue = posix_ipc.MessageQueue(QUEUE_NAME, read=False)
try:
    queue.send("start of message\0rest of message after \\0")
finally:
    queue.close()
