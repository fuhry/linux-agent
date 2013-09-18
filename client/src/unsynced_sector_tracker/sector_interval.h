#ifndef DATTO_CLIENT_UNSYNCED_SECTOR_TRACKER_SECTOR_INTERVAL_H_
#define DATTO_CLIENT_UNSYNCED_SECTOR_TRACKER_SECTOR_INTERVAL_H_

#include <boost/icl/interval.hpp>

namespace datto_linux_client {

// A useful constructor for this is: SectorInterval(start, end);
// This creates [start, end) -- notice the exclusive end bound
// e.g. SectorInterval(1, 5) = {1, 2, 3, 4}
typedef boost::icl::interval<uint64_t>::type SectorInterval;

}

#endif //  DATTO_CLIENT_UNSYNCED_SECTOR_TRACKER_SECTOR_INTERVAL_H_
