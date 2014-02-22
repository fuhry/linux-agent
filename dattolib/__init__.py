#!/usr/bin/env python

import sys
import os
import socket
import struct
from request_pb2 import Request
from reply_pb2 import Reply


IPC_SOCKET_PATH = "/var/lib/datto/dattod_ipc"

class DattodConnectionError(Exception):
    pass

# Send a Request to dattod over the unix socket IPC_SOCKET_PATH
def make_request_to_dattod(request):
    # check if socket path exists, bail if not
    if not os.path.exists(IPC_SOCKET_PATH):
        print 'Abort: Socket does not exist!'
        raise Exception("Socket does not exist")

    request_serialized = request.SerializeToString()

    try:
        s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        s.connect(IPC_SOCKET_PATH)

        s.send(struct.pack('!I', len(request_serialized)))
        s.send(request_serialized)

        # receive 4 bytes containing the size of the Reply
        length = s.recv(4)
        length = struct.unpack('!I', length)[0]

        # Create the reply
        reply = Reply()
        # Get the data from the network
        data = s.recv(length)
        # Interpret the data as a Reply object
        reply.ParseFromString(data)
        return reply
    except socket.error as e:
        raise DattodConnectionError("Unable to connect to dattod: " + str(e))
    finally:
        s.close()
