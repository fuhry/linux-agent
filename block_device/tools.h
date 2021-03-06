#ifndef DATTO_CLIENT_BLOCK_DEVICE_TOOLS_H_
#define DATTO_CLIENT_BLOCK_DEVICE_TOOLS_H_

/**
	File:        tools.h
	Author:      Charles Roydhouse
	Description: Useful macros for filesystem reads
*/

#define _swapbits16(x) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))

#define letobe16(x) _swapbits16(x)
#define letobe32(x) ((((x) & 0xff) << 24)    | (((x) & 0xff00) << 8) | \
                     (((x) & 0xff0000) >> 8) | (((x) >> 24) & 0xff))

#define betole16(x) _swapbits16(x)
#define betole32(x) (((x >> 24) & 0xff) | ((x << 8) & 0xff0000) | \
                    ((x >> 8) & 0xff00) | ((x << 24) & 0xff000000))

#define my_roundup(n, m) m * (((n - 1) / m) + 1)
#define my_rounddown(n, m) ((n/m) * m)

#endif //  DATTO_CLIENT_BLOCK_DEVICE_TOOLS_H_
