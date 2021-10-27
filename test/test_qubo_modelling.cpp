#include "utils/test_common.hpp"
#include "utils/qubo_modelling.hpp"

using namespace majorminer;

#define PENALTY ((qcoeff_t)10000)

#define ROSENBERG_TEST(x1Neg, x2Neg)                      \
  QModel model{};                                         \
  auto vars = model.createBinaryVars(2);                  \
  model.addRosenbergPolynomial<x1Neg, x2Neg>(vars[0], vars[1], PENALTY); \
  auto reformulated = model.reformulate()

TEST(QuboReformulation, Rosenberg)
{
  ROSENBERG_TEST(false, false);
  QEnumerationVerifier verifier {model, reformulated, PENALTY, true, true};
  ASSERT_TRUE(verifier.verify());
}

TEST(QuboReformulation, PartialLeftNegatedRosenberg)
{
  ROSENBERG_TEST(true, false);
  QEnumerationVerifier verifier {model, reformulated, PENALTY, true};
  ASSERT_TRUE(verifier.verify());
}

TEST(QuboReformulation, PartialRightNegatedRosenberg)
{
  ROSENBERG_TEST(false, true);
  QEnumerationVerifier verifier {model, reformulated, PENALTY, true, true};
  ASSERT_TRUE(verifier.verify());
}

TEST(QuboReformulation, NegatedRosenberg)
{
  ROSENBERG_TEST(true, true);
  QEnumerationVerifier verifier {model, reformulated, PENALTY, true};
  ASSERT_TRUE(verifier.verify());
}

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
  majorminer::assertGEQ1(8, PENALTY);
}
