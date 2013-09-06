#ifndef DATTO_CLIENT_BLOCK_TRACER_INTERVAL_QUEUE_H_
#define DATTO_CLIENT_BLOCK_TRACER_INTERVAL_QUEUE_H_

#include <queue>
#include <boost/icl/interval.hpp>

namespace datto_linux_client {

typedef std::queue<boost::icl::interval<int>> IntervalQueue;

}

#endif //  DATTO_CLIENT_BLOCK_TRACER_INTERVAL_QUEUE_H_
