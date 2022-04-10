#include <gtest/gtest.h>
#include <majorminer.hpp>
#include <common/embedding_analyzer.hpp>
#include <common/graph_gen.hpp>

#include "utils/test_common.hpp"
#include "utils/qubo_problems.hpp"

using namespace majorminer;

TEST(PerfTest, CompleteLarge_21)
{
  graph_t clique = generate_completegraph(21);
  graph_t chimera = generate_chimera(11, 11);
  EmbeddingSuite suite{clique, chimera};
  auto embedding = suite.find_embedding();
  ASSERT_TRUE(suite.connectsNodes());
}


