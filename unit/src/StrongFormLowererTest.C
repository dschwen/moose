// Unit tests for StrongFormLowerer (IBP lowering for strong forms)

#include "gtest/gtest.h"

#include "StringExpressionParser.h"
#include "StrongFormLowerer.h"

using namespace moose::automatic_weak_form;

class StrongFormLowererTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    dim = 2;
    parser = std::make_unique<StringExpressionParser>(dim);
    // Define variables
    parser->defineVariable("u");
    // Parameters
    parser->setParameter("k", 2.0);
  }

  unsigned int dim;
  std::unique_ptr<StringExpressionParser> parser;
};

TEST_F(StrongFormLowererTest, LaplacianSimple)
{
  // g = laplacian(u)
  auto expr = parser->parse("laplacian(u)");
  StrongFormLowerer lowerer(dim);
  auto L = lowerer.lowerToWeak(expr);

  ASSERT_NE(L.c1, nullptr);
  // c1 should be grad(u)
  EXPECT_EQ(L.c1->toString(), "grad(u)");
  // c0 should be 0
  ASSERT_NE(L.c0, nullptr);
  EXPECT_EQ(L.c0->toString(), "0.000000");
}

TEST_F(StrongFormLowererTest, LaplacianWithParameter)
{
  // g = k * laplacian(u), k is constant parameter
  auto expr = parser->parse("k * laplacian(u)");
  StrongFormLowerer lowerer(dim);
  auto L = lowerer.lowerToWeak(expr);

  ASSERT_NE(L.c1, nullptr);
  // Expect c1 to contain k and grad(u)
  std::string c1 = L.c1->toString();
  EXPECT_TRUE(c1.find("grad(u)") != std::string::npos);
  EXPECT_TRUE(c1.find("k") != std::string::npos);

  // c0 should be zero since grad(k)=0
  ASSERT_NE(L.c0, nullptr);
  EXPECT_EQ(L.c0->toString(), "0.000000");
}

TEST_F(StrongFormLowererTest, DivergenceOfScaledGradient)
{
  // g = div(k * grad(u))
  auto expr = parser->parse("div(k * grad(u))");
  StrongFormLowerer lowerer(dim);
  auto L = lowerer.lowerToWeak(expr);

  ASSERT_NE(L.c1, nullptr);
  std::string c1 = L.c1->toString();
  // Expect c1 = k*grad(u)
  EXPECT_TRUE(c1.find("grad(u)") != std::string::npos);
  EXPECT_TRUE(c1.find("k") != std::string::npos);

  // c0 zero because grad(k)=0
  ASSERT_NE(L.c0, nullptr);
  EXPECT_EQ(L.c0->toString(), "0.000000");
}

TEST_F(StrongFormLowererTest, NegativeDivergence)
{
  // g = -div(k * grad(u)) should lower to c1 = k*grad(u)
  auto expr = parser->parse("-div(k * grad(u))");
  StrongFormLowerer lowerer(dim);
  auto L = lowerer.lowerToWeak(expr);

  ASSERT_NE(L.c1, nullptr);
  std::string c1 = L.c1->toString();
  EXPECT_TRUE(c1.find("grad(u)") != std::string::npos);
  EXPECT_TRUE(c1.find("k") != std::string::npos);

  // c0 zero because grad(k)=0
  ASSERT_NE(L.c0, nullptr);
  EXPECT_EQ(L.c0->toString(), "0.000000");
}
