# IPC Protocol

The protocol is really simple. The first four bytes are the length of the message (in network byte order), and the rest is the message.

e.g.

    00 00 00 05 48 65 6C 6C 6F

corresponds to the ASCII string "Hello". Note that the messages passed back and forth are protobuf messages, so they will have more complicated binary representations,. Fortunately, these representations are handled by the protobuf tools and we don't need to know the implementation details.

