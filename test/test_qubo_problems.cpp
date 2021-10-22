
#include "test_common.hpp"
#include "qubo_problems.hpp"

using namespace majorminer;


TEST(QuboProblem, Simple_TSP_10)
{
  auto graph = majorminer::quboTSP(10, [](fuint32_t i, fuint32_t j){ return 0; });
  containsEdges(graph, {{0, 10}, {1, 11}, {0, 1}, {0, 2}});
}