#include "utils/test_common.hpp"
#include "utils/qubo_modelling.hpp"

using namespace majorminer;

#define PENALTY ((qcoeff_t)10000)

TEST(QuboReformulation, Enumerate_Simple_4_LEQ_1)
{
  majorminer::assertLEQ1(4, PENALTY);
}


TEST(QuboReformulation, Enumerate_Simple_9_LEQ_1)
{
  majorminer::assertLEQ1(9, PENALTY);
}

TEST(QuboReformulation, Enumerate_Simple_4_EQ_1)
{
  majorminer::assertEquality1(4, PENALTY);
}

TEST(QuboReformulation, Enumerate_Simple_9_EQ_1)
{
  majorminer::assertEquality1(4, PENALTY);
}

TEST(QuboReformulation, Enumerate_Simple_4_Absorber)
{
  majorminer::assertAbsorber(4, PENALTY);
}

TEST(QuboReformulation, Enumerate_Simple_9_Absorber)
{
  majorminer::assertAbsorber(4, PENALTY);
}