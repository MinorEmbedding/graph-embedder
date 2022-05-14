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



TEST(PerfTest, DISABLED_CompleteGraphTest_Frequencies_Chimera)
{
  graph_t chimera = generate_chimera(16, 16);
  for (fuint32_t n = 1; n < 35; ++n)
  {
    graph_t clique = generate_completegraph(n);
    fuint32_t nbValid = 0;
    for (fuint32_t i = 0; i < 100;)
    {
      EmbeddingSuite suite{clique, chimera};
      auto embedding = suite.find_embedding();
      if (suite.connectsNodes())
      {
        i++; // in case of a bug, skip this attempt
        nbValid++;
      }
    }
    std::cout << n << ":" << nbValid << std::endl;
  }
}
