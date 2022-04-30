#include <gtest/gtest.h>
#include <majorminer.hpp>
#include <common/embedding_visualizer.hpp>
#include <common/embedding_analyzer.hpp>
#include <common/graph_gen.hpp>
#include <common/debug_utils.hpp>
#include <lmrp/lmrp_chimera_subgraph.hpp>
#include <lmrp/lmrp_heuristic.hpp>

#include "utils/test_common.hpp"
#include "utils/state_gen.hpp"

using namespace majorminer;


TEST(LMRPTest, SimpleChimera)
{
  auto chimera = majorminer::generate_chimera(3, 3);
  ChimeraGraphInfo info{3,3};
  ChimeraLMRPSubgraph subgraph{info};
  graph_t trivial{ { 0, 1 } };
  StateGen gen{trivial, chimera};
  gen.addMapping(0, { 3, 7, 12, 13, 14, 15, 8, 32, 37});
  gen.addMapping(1, {45});
  auto state = gen.get();
  state->setLMRPSubgraphGenerator(&subgraph);
  LMRPHeuristic lmrp{*state, 32};
  //lmrp.optimize();
  auto repaired = lmrp.getMapping();
  for (auto mapped : repaired)
  {
    std::cout << mapped.first << " --> " << mapped.second << std::endl;
  }
}
