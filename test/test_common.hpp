#ifndef __MAJORMINER_TEST_COMMON_HPP_
#define __MAJORMINER_TEST_COMMON_HPP_

#include <gtest/gtest.h>
#include <tbb/parallel_for.h>
#include <majorminer.hpp>

namespace majorminer
{
  void containsEdges(const graph_t& graph, std::initializer_list<edge_t> edges);

  void printGraph(const graph_t& graph);

}




#endif
