#include <gtest/gtest.h>
#include <majorminer.hpp>

using namespace majorminer;


TEST(SimpleTBBTest, BasicSort)
{
  auto vec = majorminer::testTBB();
  for (size_t i = 0; i < vec.size(); ++i)
  {
    if (i + 1 < vec.size()) { EXPECT_LE(vec[i], vec[i + 1]); }
  }
}


TEST(EmbeddingTest, Basic)
{
  graph_t cycle = generate_cyclegraph(4);
  graph_t chimera = generate_chimera(1, 1);
  EmbeddingSuite suite{cycle, chimera};
  auto embedding = suite.find_embedding();
}