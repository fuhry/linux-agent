#include <boost/icl/interval_set.hpp>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>

typedef boost::icl::interval_set<uint32_t> SectorSet;

inline uint32_t rand_32() {
  uint32_t x = rand() & 0xff;

  x |= (rand() & 0xff) << 8;
  x |= (rand() & 0xff) << 16;
  x |= (rand() & 0xff) << 24;

  return x;
}

int main(void) {
  // 2^40 = 1TB in bytes
  // 2^12 = 4096, traced block size
  // Then, 2^28 is the number of blocks in a TB
  //
  // We will random write half a terabyte worth of data
  // So, 2^27 iterations

  uint32_t length = pow(2, 27);
  std::cout << "Length: " << length << std::endl;

  SectorSet block_set;

  for (uint32_t i = 0; i < length; i += 1) {
    block_set.add(rand_32() % int(pow(2, 31)));

    if (!(i % int(pow(2, 25)))) {
      std::cout << i << std::endl;
    }

  }

  std::ifstream f;
  f.open("/proc/self/status");

  while (f.good()) {
    std::string line;
    getline(f, line);
    std::cout << line << std::endl;
  }
  return 0;
}
