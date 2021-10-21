#ifndef __MAJORMINER_MAJOR_MINER_HPP_
#define __MAJORMINER_MAJOR_MINER_HPP_

#include<tbb/tbb.h>
#include<tbb/parallel_sort.h>
#include<vector>
#include<iostream>
#include<Eigen/Sparse>

#include "config.hpp"
#include "graph_gen.hpp"
#include "utils.hpp"


namespace majorminer
{

  bool works();
  std::vector<int> testTBB();

  class EmbeddingSuite
  {
    public:
      EmbeddingSuite(const graph_t& source, const graph_t& target);

    private:
      adjacency_list_t m_source;
      adjacency_list_t m_target;
  };

}



#endif