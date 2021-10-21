#include "test_common.hpp"

using namespace majorminer;




TEST(ChimeraGen, Simple_Chimera)
{
  auto graph = majorminer::generate_chimera(1,1);
  EXPECT_EQ(graph.size(), 16);
  containsEdges(graph, {
    {0, 4}, {0, 5}, {0, 6}, {0, 7},
    {1, 4}, {1, 5}, {1, 6}, {1, 7},
    {2, 4}, {2, 5}, {2, 6}, {2, 7},
    {3, 4}, {3, 5}, {3, 6}, {3, 7}
  });
}

TEST(ChimeraGen, Chimera_1x2)
{
  auto graph = majorminer::generate_chimera(1,2);
  EXPECT_EQ(graph.size(), 36);
  containsEdges(graph, {
    {4, 12}, {5, 13}, {6, 14}, {7, 15}
  });
}

TEST(ChimeraGen, Chimera_2x1)
{
  auto graph = majorminer::generate_chimera(2, 1);
  EXPECT_EQ(graph.size(), 36);
  containsEdges(graph, {
    {0, 8}, {1, 9}, {2, 10}, {3, 11}
  });
}

TEST(KingGraphGen, King_3x3)
{
  auto graph = majorminer::generate_king(3, 3);
  EXPECT_EQ(graph.size(), 20);
  containsEdges(graph, {
    {0, 1}, {1, 2},
    {3, 4}, {4, 5},
    {6, 7}, {7, 8},
    {0, 3}, {3, 6},
    {1, 4}, {4, 7},
    {2, 5}, {5, 8},

    {0, 4}, {1, 3},
    {1, 5}, {2, 4},
    {3, 7}, {4, 6},
    {4, 8}, {5, 7}
  });
}

TEST(ImportGraphEdgeList, SimpleEdgeList)
{
  auto graph = majorminer::import_graph("test/data/sample_edgelists/simple_edgelist.txt");
  EXPECT_EQ(graph.size(), 3);
  containsEdges(graph, {
    {0,1},
    {1,2},
    {2,0}
  });
}

TEST(ImportGraphEdgeList, EmptyEdgeList)
{
  auto graph = majorminer::import_graph("test/data/sample_edgelists/empty_edgelist.txt");
  EXPECT_EQ(graph.size(), 0);
}