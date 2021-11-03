#include <gtest/gtest.h>
#include <majorminer.hpp>
#include <common/embedding_visualizer.hpp>
#include <common/embedding_analyzer.hpp>

#include "utils/test_common.hpp"
#include "utils/qubo_problems.hpp"

using namespace majorminer;


TEST(EmbeddingTest, Basic_Cycle_4)
{
  graph_t cycle = generate_cyclegraph(4);
  graph_t chimera = generate_chimera(1, 1);
  auto visualizer = std::make_unique<ChimeraVisualizer>(cycle, chimera, "imgs/Basic_Cycle_4/chimera_cycle_4", 1, 1);
  EmbeddingSuite suite{cycle, chimera};
  auto embedding = suite.find_embedding();
  ASSERT_TRUE(suite.isValid());
}

TEST(EmbeddingTest, Cycle_5_Extra_Edges)
{
  graph_t cycle = generate_cyclegraph(5);
  graph_t chimera = generate_chimera(1, 1);
  addEdges(cycle, { {0,2}, {2, 4}, {1,3}});
  auto visualizer = std::make_unique<ChimeraVisualizer>(cycle, chimera, "imgs/Cycle_5_Extra_Edges/chimera_cycle_5_ExtraEdges", 1, 1);
  EmbeddingSuite suite{cycle, chimera};
  auto embedding = suite.find_embedding();
  ASSERT_TRUE(suite.isValid());
}

TEST(EmbeddingTest, Cycle_5_Extra_Edges_On_2_2_Chimera)
{
  graph_t cycle = generate_cyclegraph(5);
  graph_t chimera = generate_chimera(2, 2);
  addEdges(cycle, { {0,2}, {2, 4}, {1,3}});
  auto visualizer = std::make_unique<ChimeraVisualizer>(cycle, chimera, "imgs/Cycle_5_Extra_Edges_On_2_2_Chimera/chimera_cycle_5_ExtraEdges_Larger", 2, 2);
  EmbeddingSuite suite{cycle, chimera, visualizer.get()};
  auto embedding = suite.find_embedding();
  ASSERT_TRUE(suite.isValid());
}

TEST(EmbeddingTest, Complete_Graph_8_On_3_3_Chimera)
{
  graph_t clique = generate_completegraph(8);
  graph_t chimera = generate_chimera(3, 3);
  auto visualizer = std::make_unique<ChimeraVisualizer>(clique, chimera, "imgs/Complete_Graph_8_On_3_3_Chimera/chimera_clique_8", 3, 3);
  EmbeddingSuite suite{clique, chimera, visualizer.get()};
  auto embedding = suite.find_embedding();
  ASSERT_TRUE(suite.connectsNodes());
}

TEST(EmbeddingTest, Basic_Cycle_8_Visualization)
{
  graph_t cycle = generate_cyclegraph(8);
  graph_t chimera = generate_chimera(2, 2);
  auto visualizer = std::make_unique<ChimeraVisualizer>(cycle, chimera, "imgs/Basic_Cycle_8_Visualization/chimera_cycle_8", 2, 2);
  EmbeddingSuite suite{cycle, chimera, visualizer.get()};
  auto embedding = suite.find_embedding();
}

TEST(EmbeddingTest, Petersen_Chimera)
{
  graph_t petersen = generate_petersen();
  graph_t chimera = generate_chimera(2, 2);
  auto visualizer = std::make_unique<ChimeraVisualizer>(petersen, chimera, "imgs/Petersen_Chimera/chimera_petersen", 2, 2);
  EmbeddingSuite suite{petersen, chimera, visualizer.get()};
  auto embedding = suite.find_embedding();
}


TEST(EmbeddingTest, Petersen_KingsGraph)
{
  graph_t petersen = generate_petersen();
  graph_t king = generate_king(10, 10);
  auto visualizer = std::make_unique<KingsVisualizer>(petersen, king, "imgs/Petersen_KingsGraph/king_petersen", 10, 10);
  EmbeddingSuite suite{petersen, king, visualizer.get()};
  auto embedding = suite.find_embedding();
}

TEST(EmbeddingTest, DISABLED_TSP_7)
{
  graph_t tsp = majorminer::quboTSP(7, [](fuint32_t, fuint32_t){ return 1;});
  printGraph(tsp);
  graph_t chimera = generate_chimera(16, 16);
  auto visualizer = std::make_unique<ChimeraVisualizer>(tsp, chimera, "imgs/TSP_7/tsp_7_chimera_2000x", 16, 16);
  EmbeddingSuite suite{tsp, chimera, visualizer.get()};
  auto embedding = suite.find_embedding();
  EmbeddingAnalyzer analyzer{embedding};
  std::cout << analyzer.getNbOverlaps() << " nodes overlap and "
            << analyzer.getNbUsedNodes()
            << " nodes were needed." << std::endl;
}
