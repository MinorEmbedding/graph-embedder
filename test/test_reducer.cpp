#include <gtest/gtest.h>
#include <majorminer.hpp>
#include <common/graph_gen.hpp>
#include <common/embedding_visualizer.hpp>
#include <initial/csc_evolutionary.hpp>

#include "utils/state_gen.hpp"

using namespace majorminer;

namespace
{
  embedding_mapping_t runEmbedding(const graph_t& source, const graph_t& target)
  {
    EmbeddingSuite suite{source, target};
    embedding_mapping_t sol = suite.find_embedding();
    // ASSERT_TRUE(suite.connectsNodes());
    return sol;
  }

  std::unique_ptr<EmbeddingState> getState(const graph_t& source, const graph_t& target)
  {
    embedding_mapping_t mapping = runEmbedding(source, target);
    StateGen gen {source, target};
    gen.addMapping(mapping);
    return gen.get();
  }
}

TEST(ReducerTest, SimpleEvoReducer)
{
  graph_t clique = majorminer::generate_completegraph(12);
  graph_t chimera = majorminer::generate_chimera(4,4);
  auto visualizer = std::make_unique<ChimeraVisualizer>(clique, chimera, "imgs/SimpleEvoReducer/SimpleEvoReducer", 4, 4);

  auto state = getState(clique, chimera);
  EvolutionaryCSCReducer reducer{*state.get(), 3};
  reducer.setVisualizer(visualizer.get());
  reducer.optimize();
}
