#include <gtest/gtest.h>
#include <majorminer.hpp>
#include <common/embedding_visualizer.hpp>
#include <common/embedding_analyzer.hpp>
#include <common/graph_gen.hpp>
#include <common/debug_utils.hpp>
#include <lmrp/lmrp_chimera_subgraph.hpp>
#include <lmrp/lmrp_king_subgraph.hpp>
#include <lmrp/lmrp_heuristic.hpp>

#include "utils/test_common.hpp"
#include "utils/state_gen.hpp"

using namespace majorminer;


TEST(LMRPTest, SimpleChimera)
{
  auto chimera = majorminer::generate_chimera(3, 3);
  ChimeraGraphInfo info{3,3};
  ChimeraLMRPSubgraph subgraph{info};
  graph_t trivial{ { 0, 1 }, {0, 2} };
  StateGen gen{trivial, chimera};
  gen.addMapping(0, { 3, 7, 12, 13, 14, 15, 8, 32, 56, 62, 70});
  gen.addMapping(1, {37, 45});
  gen.addMapping(2, {36});
  auto state = gen.get();
  state->setLMRPSubgraphGenerator(&subgraph);
  LMRPHeuristic lmrp{*state, 32};
  lmrp.optimize();
  auto repaired = lmrp.getMapping();
  for (auto mapped : repaired)
  {
    std::cout << mapped.first << " --> " << mapped.second << std::endl;
  }
}


TEST(LMRPTest, SimpleKings)
{
  auto chimera = majorminer::generate_king(7, 7);
  KingGraphInfo info{7,7};
  KingLMRPSubgraph subgraph{info};
  graph_t trivial{ }; //{ 0, 1 }, {0, 2}, {1, 2}};
  StateGen gen{trivial, chimera};
  gen.addMapping(0, { 21,22,23,31,25,26,27});
  //gen.addMapping(1, { 3, 10, 17, 24, 30, 38, 45});
  //gen.addMapping(2, {0, 8, 16});
  auto state = gen.get();
  state->setLMRPSubgraphGenerator(&subgraph);
  LMRPHeuristic lmrp{*state, 24};
  lmrp.optimize();
  auto repaired = lmrp.getMapping();
  for (auto mapped : repaired)
  {
    std::cout << mapped.first << " --> " << mapped.second << std::endl;
  }
}