#include <gtest/gtest.h>
#include <majorminer.hpp>
#include <common/graph_gen.hpp>
#include <common/embedding_visualizer.hpp>
#include <common/embedding_manager.hpp>
#include <common/embedding_validator.hpp>
#include <common/utils.hpp>
#include <initial/csc_evolutionary.hpp>

#include "utils/state_gen.hpp"

using namespace majorminer;

namespace
{
  void getWorstSource(const embedding_mapping_t& mapping, vertex_t& source)
  {
    embedding_mapping_t reverse{};
    for (const auto& mapped : mapping) reverse.insert(std::make_pair(mapped.second, mapped.first));
    VertexNumberMap stats{};
    for (const auto& rev : reverse) stats[rev.second] = 0;
    for (const auto& rev : reverse)
    {
      if (reverse.count(rev.first) >= 2) stats[rev.second]++;
    }
    fuint32_t worstNumber = 0;
    for (const auto& stat : stats)
    {
      if (stat.second >= worstNumber)
      {
        worstNumber = stat.second;
        source = stat.first;
      }
    }
    std::cout << "Worst is " << source << std::endl;
  }

  void runTest(const graph_t& source, const graph_t& target, EmbeddingVisualizer* visualizer, vertex_t sourceVertex)
  {
    EmbeddingSuite suite{source, target};
    embedding_mapping_t mapping = suite.find_embedding();
    ASSERT_TRUE(suite.connectsNodes());
    StateGen gen {source, target};
    gen.addMapping(mapping);
    auto state = gen.get();

    if (!isDefined(sourceVertex)) getWorstSource(mapping, sourceVertex);
    EvolutionaryCSCReducer reducer{*state, sourceVertex};
    reducer.setVisualizer(visualizer);
    reducer.optimize();

    StateGen adjusted{source, target};
    adjusted.addMapping(mapping);
    adjusted.removeSuperVertex(sourceVertex);
    adjusted.addMapping(sourceVertex, reducer.getPlacement());
    auto newState = adjusted.get();
    EmbeddingValidator validator{*newState};
    ASSERT_TRUE(validator.nodesConnected());
  }
}

TEST(ReducerTest, SimpleEvoReducer)
{
  graph_t clique = majorminer::generate_completegraph(18);
  graph_t chimera = majorminer::generate_chimera(8,8);
  auto visualizer = std::make_unique<ChimeraVisualizer>(clique, chimera, "imgs/SimpleEvoReducer/SimpleEvoReducer", 8,8);
  runTest(clique, chimera, visualizer.get(), FUINT32_UNDEF);
}
