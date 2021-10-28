#include <gtest/gtest.h>
#include <majorminer.hpp>

#include "utils/test_common.hpp"

using namespace majorminer;


TEST(EmbeddingTest, Basic_Cycle_4)
{
  graph_t cycle = generate_cyclegraph(4);
  graph_t chimera = generate_chimera(1, 1);
  printGraph(cycle);
  EmbeddingSuite suite{cycle, chimera};
  auto embedding = suite.find_embedding();
}

TEST(EmbeddingTest, Cycle_5_Extra_Edges)
{
  graph_t cycle = generate_cyclegraph(5);
  graph_t chimera = generate_chimera(1, 1);
  addEdges(cycle, { {0,2}, {2, 4}, {1,3}});
  printGraph(cycle);
  EmbeddingSuite suite{cycle, chimera};
  auto embedding = suite.find_embedding();
}

TEST(EmbeddingTest, Cycle_5_Extra_Edges_On_2_2_Chimera)
{
  graph_t cycle = generate_cyclegraph(5);
  graph_t chimera = generate_chimera(2, 2);
  addEdges(cycle, { {0,2}, {2, 4}, {1,3}});
  printGraph(cycle);
  EmbeddingSuite suite{cycle, chimera};
  auto embedding = suite.find_embedding();
}