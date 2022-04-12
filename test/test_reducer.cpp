#include <gtest/gtest.h>
#include <majorminer.hpp>
#include <common/graph_gen.hpp>
#include <common/embedding_visualizer.hpp>
#include <common/embedding_manager.hpp>
#include <common/embedding_validator.hpp>
#include <initial/csc_evolutionary.hpp>

#include "utils/state_gen.hpp"

using namespace majorminer;

namespace
{
  void runTest(const graph_t& source, const graph_t& target, EmbeddingVisualizer* visualizer, vertex_t sourceVertex)
  {
    EmbeddingSuite suite{source, target};
    embedding_mapping_t mapping = suite.find_embedding();
    ASSERT_TRUE(suite.connectsNodes());
    StateGen gen {source, target};
    gen.addMapping(mapping);
    auto state = gen.get();

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
  graph_t clique = majorminer::generate_completegraph(12);
  graph_t chimera = majorminer::generate_chimera(4,4);
  auto visualizer = std::make_unique<ChimeraVisualizer>(clique, chimera, "imgs/SimpleEvoReducer/SimpleEvoReducer", 4, 4);
  runTest(clique, chimera, visualizer.get(), 3);
}
