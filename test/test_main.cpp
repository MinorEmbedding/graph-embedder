#include <gtest/gtest.h>
#include <majorminer.hpp>

using namespace majorminer;

TEST(SimpleTest, Basic)
{
  EXPECT_TRUE(majorminer::works());
}
