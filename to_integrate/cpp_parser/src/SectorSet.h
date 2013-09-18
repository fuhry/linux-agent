#ifndef DATTO_CLIENT_UNSYNCED_BLOCK_TRACKER_SECTOR_SET_H_
#define DATTO_CLIENT_UNSYNCED_BLOCK_TRACKER_SECTOR_SET_H_

#include <boost/icl/interval_set.hpp>
#include <stdint.h>

namespace datto_linux_client {

typedef boost::icl::interval_set<uint64_t> SectorSet;

}

#endif //  DATTO_CLIENT_UNSYNCED_BLOCK_TRACKER_SECTOR_SET_H_
