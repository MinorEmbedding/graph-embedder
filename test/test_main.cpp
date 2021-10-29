#include <gtest/gtest.h>
#include <majorminer.hpp>
#include <embedding_visualizer.hpp>

#include "utils/test_common.hpp"

using namespace majorminer;


TEST(EmbeddingTest, Basic_Cycle_4)
{
  graph_t cycle = generate_cyclegraph(4);
  graph_t chimera = generate_chimera(1, 1);
  EmbeddingSuite suite{cycle, chimera};
  auto embedding = suite.find_embedding();
  ASSERT_TRUE(suite.isValid());
}

TEST(EmbeddingTest, Cycle_5_Extra_Edges)
{
  graph_t cycle = generate_cyclegraph(5);
  graph_t chimera = generate_chimera(1, 1);
  addEdges(cycle, { {0,2}, {2, 4}, {1,3}});
  EmbeddingSuite suite{cycle, chimera};
  auto embedding = suite.find_embedding();
  ASSERT_TRUE(suite.isValid());
}

TEST(EmbeddingTest, Cycle_5_Extra_Edges_On_2_2_Chimera)
{
  graph_t cycle = generate_cyclegraph(5);
  graph_t chimera = generate_chimera(2, 2);
  addEdges(cycle, { {0,2}, {2, 4}, {1,3}});
  EmbeddingSuite suite{cycle, chimera};
  auto embedding = suite.find_embedding();
  ASSERT_TRUE(suite.isValid());
}

TEST(EmbeddingTest, Complete_Graph_8_On_3_3_Chimera)
{
  graph_t clique = generate_completegraph(8);
  graph_t chimera = generate_chimera(3, 3);
  EmbeddingSuite suite{clique, chimera};
  auto embedding = suite.find_embedding();
  ASSERT_TRUE(suite.connectsNodes());
  ChimeraVisualizer visualizer{clique, chimera, "chimera_clique_8", 3, 3};
  visualizer.draw(embedding);
}

TEST(EmbeddingTest, Basic_Cycle_8_Visualization)
{
  graph_t cycle = generate_cyclegraph(8);
  graph_t chimera = generate_chimera(2, 2);
  EmbeddingSuite suite{cycle, chimera};
  auto embedding = suite.find_embedding();
  ChimeraVisualizer visualizer{cycle, chimera, "chimera_cycle_8", 2, 2};
  visualizer.draw(embedding);
}

TEST(EmbeddingTest, Petersen_Chimera)
{
  graph_t petersen = generate_petersen();
  graph_t chimera = generate_chimera(2, 2);
  EmbeddingSuite suite{petersen, chimera};
  auto embedding = suite.find_embedding();
  ChimeraVisualizer visualizer{petersen, chimera, "chimera_petersen", 2, 2};
  visualizer.draw(embedding);
}


TEST(EmbeddingTest, Petersen_KingsGraph)
{
  graph_t petersen = generate_petersen();
  graph_t king = generate_king(10, 10);
  EmbeddingSuite suite{petersen, king};
  auto embedding = suite.find_embedding();
  KingsVisualizer visualizer{petersen, king, "king_petersen", 10, 10};
  visualizer.draw(embedding);
}
