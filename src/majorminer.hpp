#ifndef __MAJORMINER_MAJOR_MINER_HPP_
#define __MAJORMINER_MAJOR_MINER_HPP_

#include <tbb/tbb.h>
#include <tbb/parallel_sort.h>

#include <vector>
#include "graph_gen.hpp"

namespace majorminer
{

  bool works();
  std::vector<int> testTBB();
}



#endif