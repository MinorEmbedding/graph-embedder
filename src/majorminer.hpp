#ifndef __MAJORMINER_MAJOR_MINER_HPP_
#define __MAJORMINER_MAJOR_MINER_HPP_

#include<tbb/tbb.h>
#include<tbb/parallel_sort.h>
#include<vector>
#include<iostream>

#include "config.hpp"
#include "graph_gen.hpp"

#include<Eigen/Sparse>

namespace majorminer
{

  bool works();
  std::vector<int> testTBB();

}



#endif