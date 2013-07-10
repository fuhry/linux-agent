#!/usr/bin/env python

# Helper script to test sending messages to the datto daemon (dattod)
#
# Uses http://semanchuk.com/philip/posix_ipc/
# You will need to install python-dev to install posix_ipc

import posix_ipc

QUEUE_NAME = "/dattod"

queue = posix_ipc.MessageQueue(QUEUE_NAME, read=False)
try:
    queue.send("EHello, world")
finally:
    queue.close()
