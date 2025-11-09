//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "ExpressionEvaluator.h"
#include "StringExpressionParser.h"
#include "libmesh/vector_value.h"

using namespace moose::automatic_weak_form;

class ExpressionEvaluatorTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    parser = std::make_unique<StringExpressionParser>(3);

    // Define test variables
    parser->defineVariable("u");
    parser->defineVariable("v");
    parser->defineVariable("c");

    // Define test parameters
    parser->setParameter("kappa", 0.01);
    parser->setParameter("lambda", 0.001);
  }

  std::unique_ptr<StringExpressionParser> parser;
};

// Test basic scalar expression evaluation
TEST_F(ExpressionEvaluatorTest, ScalarConstant)
{
  auto expr = parser->parse("5.0");
  ExpressionEvaluator evaluator(3);

  auto result = evaluator.evaluate(expr);
  EXPECT_NEAR(result, 5.0, 1e-12);
}

// Test simple variable
TEST_F(ExpressionEvaluatorTest, ScalarVariable)
{
  auto expr = parser->parse("u");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("u", 0);
  evaluator.setFieldValue(0, 2.5);

  auto result = evaluator.evaluate(expr);
  EXPECT_NEAR(result, 2.5, 1e-12);
}

// Test arithmetic operations
TEST_F(ExpressionEvaluatorTest, Addition)
{
  auto expr = parser->parse("u + v");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("u", 0);
  evaluator.registerVariable("v", 1);
  evaluator.setFieldValue(0, 2.0);
  evaluator.setFieldValue(1, 3.0);

  auto result = evaluator.evaluate(expr);
  EXPECT_NEAR(result, 5.0, 1e-12);
}

// Test multiplication
TEST_F(ExpressionEvaluatorTest, Multiplication)
{
  auto expr = parser->parse("u * v");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("u", 0);
  evaluator.registerVariable("v", 1);
  evaluator.setFieldValue(0, 2.0);
  evaluator.setFieldValue(1, 3.0);

  auto result = evaluator.evaluate(expr);
  EXPECT_NEAR(result, 6.0, 1e-12);
}

// Test power
TEST_F(ExpressionEvaluatorTest, Power)
{
  auto expr = parser->parse("pow(u, 2.0)");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("u", 0);
  evaluator.setFieldValue(0, 3.0);

  auto result = evaluator.evaluate(expr);
  EXPECT_NEAR(result, 9.0, 1e-12);
}

// Test double-well potential evaluation
TEST_F(ExpressionEvaluatorTest, DoubleWellPotential)
{
  auto expr = parser->parse("(u*u - 1.0)*(u*u - 1.0)");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("u", 0);

  // Test at u = 0 (maximum)
  evaluator.setFieldValue(0, 0.0);
  EXPECT_NEAR(evaluator.evaluate(expr), 1.0, 1e-12);

  // Test at u = 1 (minimum)
  evaluator.setFieldValue(0, 1.0);
  EXPECT_NEAR(evaluator.evaluate(expr), 0.0, 1e-12);

  // Test at u = -1 (minimum)
  evaluator.setFieldValue(0, -1.0);
  EXPECT_NEAR(evaluator.evaluate(expr), 0.0, 1e-12);

  // Test at u = 2
  evaluator.setFieldValue(0, 2.0);
  EXPECT_NEAR(evaluator.evaluate(expr), 9.0, 1e-12);
}

// Test gradient access
TEST_F(ExpressionEvaluatorTest, GradientAccess)
{
  auto expr = parser->parse("grad(u)");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("u", 0);
  evaluator.setFieldGradient(0, RealGradient(1.0, 2.0, 3.0));

  auto result = evaluator.evaluateVector(expr);
  EXPECT_NEAR(result(0), 1.0, 1e-12);
  EXPECT_NEAR(result(1), 2.0, 1e-12);
  EXPECT_NEAR(result(2), 3.0, 1e-12);
}

// Test vector scaling
TEST_F(ExpressionEvaluatorTest, VectorScaling)
{
  auto expr = parser->parse("2.0 * grad(u)");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("u", 0);
  evaluator.setFieldGradient(0, RealGradient(1.0, 2.0, 3.0));

  auto result = evaluator.evaluateVector(expr);
  EXPECT_NEAR(result(0), 2.0, 1e-12);
  EXPECT_NEAR(result(1), 4.0, 1e-12);
  EXPECT_NEAR(result(2), 6.0, 1e-12);
}

// Test dot product evaluation
TEST_F(ExpressionEvaluatorTest, DotProduct)
{
  auto expr = parser->parse("dot(grad(u), grad(u))");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("u", 0);
  evaluator.setFieldGradient(0, RealGradient(3.0, 4.0, 0.0));

  auto result = evaluator.evaluate(expr);
  EXPECT_NEAR(result, 25.0, 1e-12); // 3^2 + 4^2 = 25
}

// Test gradient energy evaluation
TEST_F(ExpressionEvaluatorTest, GradientEnergy)
{
  auto expr = parser->parse("0.5 * kappa * dot(grad(c), grad(c))");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("c", 0);
  evaluator.setParameter("kappa", 0.01);
  evaluator.setFieldGradient(0, RealGradient(1.0, 2.0, 3.0));

  auto result = evaluator.evaluate(expr);
  // 0.5 * 0.01 * (1 + 4 + 9) = 0.5 * 0.01 * 14 = 0.07
  EXPECT_NEAR(result, 0.07, 1e-12);
}

// Test Cahn-Hilliard energy
TEST_F(ExpressionEvaluatorTest, CahnHilliardEnergy)
{
  auto expr = parser->parse("(c*c - 1.0)*(c*c - 1.0) + 0.5*kappa*dot(grad(c), grad(c))");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("c", 0);
  evaluator.setParameter("kappa", 0.01);
  evaluator.setFieldValue(0, 0.5);
  evaluator.setFieldGradient(0, RealGradient(0.1, 0.2, 0.3));

  auto result = evaluator.evaluate(expr);

  // Bulk: (0.25 - 1)^2 = 0.5625
  Real bulk = std::pow(0.5*0.5 - 1.0, 2.0);
  // Gradient: 0.5 * 0.01 * (0.01 + 0.04 + 0.09) = 0.5 * 0.01 * 0.14 = 0.0007
  Real gradient = 0.5 * 0.01 * (0.01 + 0.04 + 0.09);

  EXPECT_NEAR(result, bulk + gradient, 1e-10);
}

// Test shape inference
TEST_F(ExpressionEvaluatorTest, ShapeInference)
{
  // Scalar shape
  {
    auto expr = parser->parse("u + v");
    EXPECT_TRUE(expr->isScalar());
    EXPECT_FALSE(expr->isVector());
    EXPECT_FALSE(expr->isTensor());
  }

  // Vector shape
  {
    auto expr = parser->parse("grad(u)");
    EXPECT_FALSE(expr->isScalar());
    EXPECT_TRUE(expr->isVector());
    EXPECT_FALSE(expr->isTensor());
  }

  // Dot product returns scalar
  {
    auto expr = parser->parse("dot(grad(u), grad(v))");
    EXPECT_TRUE(expr->isScalar());
    EXPECT_FALSE(expr->isVector());
    EXPECT_FALSE(expr->isTensor());
  }
}

// Test multi-variable expressions
TEST_F(ExpressionEvaluatorTest, MultiVariableExpressions)
{
  auto expr = parser->parse("u*v + dot(grad(u), grad(v))");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("u", 0);
  evaluator.registerVariable("v", 1);
  evaluator.setFieldValue(0, 2.0);
  evaluator.setFieldValue(1, 3.0);
  evaluator.setFieldGradient(0, RealGradient(1.0, 0.0, 0.0));
  evaluator.setFieldGradient(1, RealGradient(0.0, 1.0, 0.0));

  auto result = evaluator.evaluate(expr);
  // u*v = 6.0, dot = 0.0, total = 6.0
  EXPECT_NEAR(result, 6.0, 1e-12);
}

// Test transcendental functions
TEST_F(ExpressionEvaluatorTest, SineFunction)
{
  auto expr = parser->parse("sin(u)");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("u", 0);
  evaluator.setFieldValue(0, M_PI / 2.0);

  auto result = evaluator.evaluate(expr);
  EXPECT_NEAR(result, 1.0, 1e-12);
}

TEST_F(ExpressionEvaluatorTest, CosineFunction)
{
  auto expr = parser->parse("cos(u)");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("u", 0);
  evaluator.setFieldValue(0, M_PI);

  auto result = evaluator.evaluate(expr);
  EXPECT_NEAR(result, -1.0, 1e-12);
}

TEST_F(ExpressionEvaluatorTest, ExponentialFunction)
{
  auto expr = parser->parse("exp(u)");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("u", 0);
  evaluator.setFieldValue(0, 1.0);

  auto result = evaluator.evaluate(expr);
  EXPECT_NEAR(result, std::exp(1.0), 1e-12);
}

TEST_F(ExpressionEvaluatorTest, SquareRoot)
{
  auto expr = parser->parse("sqrt(u)");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("u", 0);
  evaluator.setFieldValue(0, 16.0);

  auto result = evaluator.evaluate(expr);
  EXPECT_NEAR(result, 4.0, 1e-12);
}

// Test parameter substitution
TEST_F(ExpressionEvaluatorTest, ParameterSubstitution)
{
  auto expr = parser->parse("kappa * u");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("u", 0);
  evaluator.setParameter("kappa", 0.01);
  evaluator.setFieldValue(0, 10.0);

  auto result = evaluator.evaluate(expr);
  EXPECT_NEAR(result, 0.1, 1e-12); // kappa = 0.01, 0.01 * 10 = 0.1
}

// Test nested expressions
TEST_F(ExpressionEvaluatorTest, NestedExpressions)
{
  auto expr = parser->parse("sin(u * v) + cos(u + v)");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("u", 0);
  evaluator.registerVariable("v", 1);
  evaluator.setFieldValue(0, M_PI / 4.0);
  evaluator.setFieldValue(1, 2.0);

  auto result = evaluator.evaluate(expr);
  Real expected = std::sin(M_PI / 4.0 * 2.0) + std::cos(M_PI / 4.0 + 2.0);
  EXPECT_NEAR(result, expected, 1e-10);
}

// Test division
TEST_F(ExpressionEvaluatorTest, Division)
{
  auto expr = parser->parse("u / v");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("u", 0);
  evaluator.registerVariable("v", 1);
  evaluator.setFieldValue(0, 10.0);
  evaluator.setFieldValue(1, 2.0);

  auto result = evaluator.evaluate(expr);
  EXPECT_NEAR(result, 5.0, 1e-12);
}

// Test subtraction
TEST_F(ExpressionEvaluatorTest, Subtraction)
{
  auto expr = parser->parse("u - v");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("u", 0);
  evaluator.registerVariable("v", 1);
  evaluator.setFieldValue(0, 10.0);
  evaluator.setFieldValue(1, 3.0);

  auto result = evaluator.evaluate(expr);
  EXPECT_NEAR(result, 7.0, 1e-12);
}

// Test complex nested gradient expression
TEST_F(ExpressionEvaluatorTest, ComplexGradientExpression)
{
  auto expr = parser->parse("u*u * dot(grad(v), grad(v)) + v*v * dot(grad(u), grad(u))");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("u", 0);
  evaluator.registerVariable("v", 1);
  evaluator.setFieldValue(0, 2.0);
  evaluator.setFieldValue(1, 3.0);
  evaluator.setFieldGradient(0, RealGradient(1.0, 0.0, 0.0));
  evaluator.setFieldGradient(1, RealGradient(0.0, 1.0, 0.0));

  auto result = evaluator.evaluate(expr);
  // u^2 * |grad(v)|^2 + v^2 * |grad(u)|^2
  // 4 * 1 + 9 * 1 = 13
  EXPECT_NEAR(result, 13.0, 1e-12);
}

// Test zero gradient
TEST_F(ExpressionEvaluatorTest, ZeroGradient)
{
  auto expr = parser->parse("dot(grad(u), grad(u))");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("u", 0);
  evaluator.setFieldGradient(0, RealGradient(0.0, 0.0, 0.0));

  auto result = evaluator.evaluate(expr);
  EXPECT_NEAR(result, 0.0, 1e-12);
}

// Test very small values
TEST_F(ExpressionEvaluatorTest, SmallValues)
{
  auto expr = parser->parse("u*u + v*v");
  ExpressionEvaluator evaluator(3);

  evaluator.registerVariable("u", 0);
  evaluator.registerVariable("v", 1);
  evaluator.setFieldValue(0, 1e-10);
  evaluator.setFieldValue(1, 2e-10);

  auto result = evaluator.evaluate(expr);
  EXPECT_NEAR(result, 5e-20, 1e-25);
}

// Test associativity
TEST_F(ExpressionEvaluatorTest, Associativity)
{
  auto expr1 = parser->parse("(u + v) + c");
  auto expr2 = parser->parse("u + (v + c)");

  ExpressionEvaluator eval1(3);
  eval1.registerVariable("u", 0);
  eval1.registerVariable("v", 1);
  eval1.registerVariable("c", 2);
  eval1.setFieldValue(0, 1.0);
  eval1.setFieldValue(1, 2.0);
  eval1.setFieldValue(2, 3.0);

  ExpressionEvaluator eval2(3);
  eval2.registerVariable("u", 0);
  eval2.registerVariable("v", 1);
  eval2.registerVariable("c", 2);
  eval2.setFieldValue(0, 1.0);
  eval2.setFieldValue(1, 2.0);
  eval2.setFieldValue(2, 3.0);

  auto result1 = eval1.evaluate(expr1);
  auto result2 = eval2.evaluate(expr2);

  EXPECT_NEAR(result1, result2, 1e-12);
  EXPECT_NEAR(result1, 6.0, 1e-12);
}
