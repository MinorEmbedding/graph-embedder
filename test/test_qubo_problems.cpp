
#include "utils/test_common.hpp"
#include "utils/qubo_problems.hpp"

using namespace majorminer;


TEST(QuboProblem, Simple_TSP_10)
{
  auto graph = majorminer::quboTSP(7, [](fuint32_t i, fuint32_t j){ return i+j; });
  containsEdges(graph, {{0, 7}, {1, 8}, {0, 1}, {0, 2}});
}