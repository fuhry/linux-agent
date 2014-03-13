#ifndef DATTO_CLIENT_UNSYNCED_SECTOR_MANAGER_TIMED_SECTOR_MAP_H_
#define DATTO_CLIENT_UNSYNCED_SECTOR_MANAGER_TIMED_SECTOR_MAP_H_

#include <boost/icl/interval_map.hpp>
#include <stdint.h>
#include <time.h>

namespace datto_linux_client {

typedef boost::icl::interval_map<uint64_t,
                                 time_t,
                                 boost::icl::partial_absorber,
                                 std::less,
                                 boost::icl::inplace_max>::type TimedSectorMap;

}

#endif //  DATTO_CLIENT_UNSYNCED_SECTOR_MANAGER_TIMED_SECTOR_MAP_H_
