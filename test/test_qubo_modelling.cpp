#include "utils/test_common.hpp"
#include "utils/qubo_modelling.hpp"

using namespace majorminer;

#define PENALTY ((qcoeff_t)10000)

TEST(QuboReformulation, Enumerate_Simple_4_LEQ_1)
{
  majorminer::assertLEQ1(4, PENALTY);
}


TEST(QuboReformulation, Enumerate_Simple_8_LEQ_1)
{
  majorminer::assertLEQ1(8, PENALTY);
}

TEST(QuboReformulation, Enumerate_Simple_4_EQ_1)
{
  majorminer::assertEquality1(4, PENALTY);
}

TEST(QuboReformulation, Enumerate_Simple_8_EQ_1)
{
  majorminer::assertEquality1(8, PENALTY);
}

TEST(QuboReformulation, Enumerate_Simple_4_Absorber)
{
  majorminer::assertAbsorber(4, PENALTY);
}

TEST(QuboReformulation, Enumerate_Simple_8_Absorber)
{
  majorminer::assertAbsorber(8, PENALTY);
}


TEST(QuboReformulation, Enumerate_Simple_9_GEQ)
{
  majorminer::assertGEQ1(9, PENALTY);
}
