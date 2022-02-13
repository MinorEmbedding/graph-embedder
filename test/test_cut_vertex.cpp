#include <common/utils.hpp>
#include <common/cut_vertex.hpp>

#include "utils/test_common.hpp"

using namespace majorminer;


namespace
{
  void expectEqualNodesets(const nodeset_t& n1, const nodeset_t& n2)
  {
    ASSERT_TRUE(n1.size() == n2.size());
    for (auto node : n1)
    {
      ASSERT_TRUE(n2.contains(node));
    }
  }
  void assertCorrectCutVertices(const graph_t& graph, nodeset_t expectedCutVertices)
  {
    adjacency_list_t adjacencyList{};
    convertToAdjacencyList(adjacencyList, graph);
    auto nodes = getNodeset(graph);

    nodeset_t cutVertices{};
    identifiyCutVertices(cutVertices, adjacencyList, nodes.size());
    expectEqualNodesets(expectedCutVertices, cutVertices);

    for (auto node : nodes)
    {
      bool expectedIsCutVertex = expectedCutVertices.contains(node);
      bool cutVertex = isCutVertex(adjacencyList, node, nodes.size());
      ASSERT_EQ(expectedIsCutVertex, cutVertex);
    }
  }
}


TEST(CutVertex, CycleGraph_8)
{
  graph_t cycle = generate_cyclegraph(8);
  assertCorrectCutVertices(cycle, {});
}

TEST(CutVertex, SimpleGraph)
{
  graph_t graph{};
  addEdges(graph, {
    {1, 2}, {1, 3}, {2, 4}, {3, 4},
    {4, 5}, {4, 6}, {5,6}});
  assertCorrectCutVertices(graph, {4});
}


TEST(CutVertex, TreeGraph)
{
  graph_t graph{};
  addEdges(graph, { // leafs 3, 4, 7, 8
    {0, 1}, {1, 2},  {1, 3}, {2, 4},
    {0, 5}, {5, 6}, {5, 7}, {6, 8}}
  );
  assertCorrectCutVertices(graph, {0, 1, 2, 5, 6});
}


TEST(CutVertex, SmallTreeGraph)
{
  graph_t graph{};
  addEdges(graph, { // leafs 2, 3, 4
    {0, 1}, {0, 2}, {1, 3}, {1, 4}
  });
  assertCorrectCutVertices(graph, {0, 1});
}



TEST(CutVertex, Clique_8)
{
  graph_t graph = generate_completegraph(8);
  assertCorrectCutVertices(graph, {});
}

TEST(CutVertex, Petersen)
{
  graph_t graph = generate_petersen();
  assertCorrectCutVertices(graph, {});
}


TEST(CutVertex, Disconnected)
{
  graph_t graph{};
  addEdges(graph, {
    {0, 1}, {1, 2}, {3,4}
  });
  ASSERT_THROW(assertCorrectCutVertices(graph, {}), std::runtime_error);
}


TEST(CutVertex, ForbiddenLoop)
{
  graph_t graph{};
  addEdges(graph, {
    {0, 0}, {1, 1}, {2, 2}, {0, 1}, {1, 2}
  });
  ASSERT_THROW(assertCorrectCutVertices(graph, { 1 }), std::runtime_error);
}



