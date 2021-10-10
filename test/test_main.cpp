#include <gtest/gtest.h>
#include <majorminer.hpp>

using namespace majorminer;

TEST(SimpleTest, Basic)
{
  EXPECT_TRUE(majorminer::works());
}

TEST(SimpleTBBTest, BasicSort)
{
  auto vec = majorminer::testTBB();
  for (auto i = 0; i < vec.size(); ++i)
  {
    if (i + 1 < vec.size()) EXPECT_LE(vec[i], vec[i + 1]);
  }
}
