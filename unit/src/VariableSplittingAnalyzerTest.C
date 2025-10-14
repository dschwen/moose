//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseAST.h"
#include "StringExpressionParser.h"
#include "VariableSplitting.h"

#include <memory>
#include <string>
#include <iostream>

using namespace moose::automatic_weak_form;

class VariableSplittingAnalyzerTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    analyzer = std::make_unique<VariableSplittingAnalyzer>(1, 3); // FE order 1, 3D
    parser = std::make_unique<StringExpressionParser>(3);
  }

  std::unique_ptr<VariableSplittingAnalyzer> analyzer;
  std::unique_ptr<StringExpressionParser> parser;

  // Helper methods
  void expectSplitVariable(const std::map<std::string, SplitVariable> & split_vars,
                           const std::string & name,
                           const std::string & original_var,
                           unsigned int derivative_order)
  {
    ASSERT_TRUE(split_vars.count(name)) << "Expected split variable '" << name << "' not found";
    const auto & sv = split_vars.at(name);
    EXPECT_EQ(sv.name, name);
    EXPECT_EQ(sv.original_variable, original_var);
    EXPECT_EQ(sv.derivative_order, derivative_order);
    EXPECT_FALSE(sv.is_primary);
    EXPECT_NE(sv.constraint_residual, nullptr);
  }

  void expectNoSplitVariable(const std::map<std::string, SplitVariable> & split_vars,
                             const std::string & name)
  {
    EXPECT_FALSE(split_vars.count(name)) << "Unexpected split variable '" << name << "' found";
  }

  bool hasVariable(const std::vector<VariableSplittingAnalyzer::SplitRequirement> & requirements,
                   const std::string & var_name,
                   unsigned int expected_order)
  {
    for (const auto & req : requirements)
    {
      if (req.variable_name == var_name && req.max_derivative_order == expected_order)
        return true;
    }
    return false;
  }
};

// Test basic derivative order analysis
TEST_F(VariableSplittingAnalyzerTest, BasicDerivativeAnalysis)
{
  // Simple gradient: grad(u) - should not require splitting for FE order 1
  {
    auto expr = parser->parse("grad(u)");
    auto requirements = analyzer->analyzeExpression(expr);

    ASSERT_EQ(requirements.size(), 1);
    EXPECT_EQ(requirements[0].variable_name, "u");
    EXPECT_EQ(requirements[0].max_derivative_order, 1);
    EXPECT_FALSE(requirements[0].requires_splitting);
  }

  // Double gradient: grad(grad(u)) - should require splitting
  {
    auto expr = parser->parse("grad(grad(u))");
    auto requirements = analyzer->analyzeExpression(expr);

    ASSERT_EQ(requirements.size(), 1);
    EXPECT_EQ(requirements[0].variable_name, "u");
    EXPECT_EQ(requirements[0].max_derivative_order, 2);
    EXPECT_TRUE(requirements[0].requires_splitting);
    ASSERT_EQ(requirements[0].split_orders.size(), 1);
    EXPECT_EQ(requirements[0].split_orders[0], 2);
  }

  // Laplacian: laplacian(u) - should require splitting (2nd order)
  {
    auto expr = parser->parse("laplacian(u)");
    auto requirements = analyzer->analyzeExpression(expr);

    ASSERT_EQ(requirements.size(), 1);
    EXPECT_EQ(requirements[0].variable_name, "u");
    EXPECT_EQ(requirements[0].max_derivative_order, 2);
    EXPECT_TRUE(requirements[0].requires_splitting);
  }
}

// Test cross-variable derivative analysis
TEST_F(VariableSplittingAnalyzerTest, CrossVariableAnalysis)
{
  // dot(grad(u), grad(v)) - both variables have 1st order derivatives
  {
    auto expr = parser->parse("dot(grad(u), grad(v))");
    auto requirements = analyzer->analyzeExpression(expr);

    EXPECT_EQ(requirements.size(), 2);
    EXPECT_TRUE(hasVariable(requirements, "u", 1));
    EXPECT_TRUE(hasVariable(requirements, "v", 1));

    // Neither should require splitting at FE order 1
    for (const auto & req : requirements)
      EXPECT_FALSE(req.requires_splitting);
  }

  // dot(grad(grad(u)), grad(v)) - mixed orders
  {
    auto expr = parser->parse("dot(grad(grad(u)), grad(v))");
    auto requirements = analyzer->analyzeExpression(expr);

    EXPECT_EQ(requirements.size(), 2);
    EXPECT_TRUE(hasVariable(requirements, "u", 2)); // Should require splitting
    EXPECT_TRUE(hasVariable(requirements, "v", 1)); // Should not require splitting

    // Check splitting requirements
    for (const auto & req : requirements)
    {
      if (req.variable_name == "u")
        EXPECT_TRUE(req.requires_splitting);
      else if (req.variable_name == "v")
        EXPECT_FALSE(req.requires_splitting);
    }
  }
}

// Test nested derivative expressions
TEST_F(VariableSplittingAnalyzerTest, NestedDerivatives)
{
  // div(grad(grad(u))) - 3rd order derivative
  {
    auto expr = parser->parse("div(grad(grad(u)))");
    auto requirements = analyzer->analyzeExpression(expr);

    ASSERT_EQ(requirements.size(), 1);
    EXPECT_EQ(requirements[0].variable_name, "u");
    EXPECT_EQ(requirements[0].max_derivative_order, 3);
    EXPECT_TRUE(requirements[0].requires_splitting);

    // Should need split variables for orders 2 and 3
    ASSERT_EQ(requirements[0].split_orders.size(), 2);
    EXPECT_EQ(requirements[0].split_orders[0], 2);
    EXPECT_EQ(requirements[0].split_orders[1], 3);
  }

  // Complex expression: laplacian(u) + grad(grad(grad(v)))
  {
    auto expr = parser->parse("laplacian(u) + grad(grad(grad(v)))");
    auto requirements = analyzer->analyzeExpression(expr);

    EXPECT_EQ(requirements.size(), 2);
    EXPECT_TRUE(hasVariable(requirements, "u", 2));
    EXPECT_TRUE(hasVariable(requirements, "v", 3));

    // Both should require splitting
    for (const auto & req : requirements)
      EXPECT_TRUE(req.requires_splitting);
  }
}

// Test split variable generation
TEST_F(VariableSplittingAnalyzerTest, SplitVariableGeneration)
{
  // Laplacian expression (2nd order)
  {
    auto expr = parser->parse("laplacian(u)");
    auto split_vars = analyzer->generateSplitVariables(expr);

    // Should generate a scalar split variable for 2nd order (Laplacian)
    expectSplitVariable(split_vars, "u_hess", "u", 2);

    // Check that the split variable is scalar (not tensor)
    const auto & sv = split_vars.at("u_hess");
    EXPECT_TRUE(std::holds_alternative<ScalarShape>(sv.shape));

    // Should have constraint equation
    EXPECT_NE(sv.constraint_residual, nullptr);
    EXPECT_NE(sv.definition, nullptr);

    // The definition should be a Laplacian node
    EXPECT_EQ(sv.definition->type(), NodeType::Laplacian);
  }

  // Third-order expression using divergence of gradient of gradient
  {
    auto expr = parser->parse("div(grad(grad(u)))");
    auto split_vars = analyzer->generateSplitVariables(expr);

    // Should generate split variables for orders 2, 3
    expectSplitVariable(split_vars, "u_hess", "u", 2);
    expectSplitVariable(split_vars, "u_d3", "u", 3);

    // Check shapes: u_hess should be scalar, u_d3 should be scalar (div of tensor)
    EXPECT_TRUE(std::holds_alternative<ScalarShape>(split_vars.at("u_hess").shape));

    // Should have constraint equations for each
    for (const auto & [name, sv] : split_vars)
    {
      EXPECT_NE(sv.constraint_residual, nullptr);
      EXPECT_NE(sv.definition, nullptr);
    }
  }

  // Mixed variables: laplacian(u) + grad(grad(v))
  {
    auto expr = parser->parse("laplacian(u) + grad(grad(v))");
    auto split_vars = analyzer->generateSplitVariables(expr);

    expectSplitVariable(split_vars, "u_hess", "u", 2);
    expectSplitVariable(split_vars, "v_hess", "v", 2);
    expectNoSplitVariable(split_vars, "u_grad");
    expectNoSplitVariable(split_vars, "v_grad");
  }
}

// Test expression transformation
TEST_F(VariableSplittingAnalyzerTest, ExpressionTransformation)
{
  // Simple Laplacian case
  {
    auto expr = parser->parse("laplacian(u)");
    auto split_vars = analyzer->generateSplitVariables(expr);
    auto transformed = analyzer->transformExpression(expr, split_vars);

    EXPECT_NE(transformed, nullptr);
    // The transformed expression should reference the split variable
    std::string transformed_str = transformed->toString();

    // laplacian(u) should transform to u_hess (scalar split variable)
    EXPECT_NE(transformed_str.find("u_hess"), std::string::npos);

    // The original expression should be different from the transformed one
    std::string original_str = expr->toString();
    EXPECT_NE(original_str, transformed_str);
  }

  // Mixed expression transformation
  {
    auto expr = parser->parse("laplacian(u) + dot(grad(v), grad(v))");
    auto split_vars = analyzer->generateSplitVariables(expr);
    auto transformed = analyzer->transformExpression(expr, split_vars);

    EXPECT_NE(transformed, nullptr);
    std::string transformed_str = transformed->toString();

    // u should be split (laplacian = 2nd order), v should not (grad = 1st order)
    if (split_vars.count("u_hess"))
      EXPECT_NE(transformed_str.find("u_hess"), std::string::npos);

    // v should not be split at FE order 1
    EXPECT_EQ(transformed_str.find("v_grad"), std::string::npos);
  }
}

// Test constraint equation generation
TEST_F(VariableSplittingAnalyzerTest, ConstraintGeneration)
{
  auto expr = parser->parse("laplacian(u)");
  auto split_vars = analyzer->generateSplitVariables(expr);
  auto constraints = analyzer->generateConstraintEquations(split_vars);

  EXPECT_EQ(constraints.size(), 1);
  EXPECT_NE(constraints[0], nullptr);

  // The constraint should be of the form: u_hess - grad(grad(u)) = 0
  std::string constraint_str = constraints[0]->toString();
  EXPECT_NE(constraint_str.find("u_hess"), std::string::npos);
}

// Test with different FE orders
TEST_F(VariableSplittingAnalyzerTest, DifferentFEOrders)
{
  // Test with FE order 2 - should not split up to 2nd derivatives
  auto analyzer_order2 = std::make_unique<VariableSplittingAnalyzer>(2, 3);
  analyzer_order2->setUseHessians(true); // Enable Hessian support for FE order 2

  {
    auto expr = parser->parse("laplacian(u)"); // 2nd order
    auto requirements = analyzer_order2->analyzeExpression(expr);

    ASSERT_EQ(requirements.size(), 1);
    EXPECT_FALSE(requirements[0].requires_splitting); // Should not split at order 2
  }

  {
    auto expr = parser->parse("grad(grad(grad(u)))"); // 3rd order
    auto requirements = analyzer_order2->analyzeExpression(expr);

    ASSERT_EQ(requirements.size(), 1);
    EXPECT_TRUE(requirements[0].requires_splitting); // Should split at order 3
  }
}

// Test requiresSplitting convenience method
TEST_F(VariableSplittingAnalyzerTest, RequiresSplittingMethod)
{
  {
    auto expr = parser->parse("grad(u)");
    EXPECT_FALSE(analyzer->requiresSplitting(expr));
  }

  {
    auto expr = parser->parse("laplacian(u)");
    EXPECT_TRUE(analyzer->requiresSplitting(expr));
  }

  {
    auto expr = parser->parse("u + v");
    EXPECT_FALSE(analyzer->requiresSplitting(expr));
  }
}

// Test canHandleWithCurrentOrder method
TEST_F(VariableSplittingAnalyzerTest, CanHandleWithCurrentOrder)
{
  {
    auto expr = parser->parse("grad(u)");
    EXPECT_TRUE(analyzer->canHandleWithCurrentOrder(expr));
  }

  {
    auto expr = parser->parse("grad(grad(u))");
    EXPECT_FALSE(analyzer->canHandleWithCurrentOrder(expr));
  }
}

// Test split variable naming consistency
TEST_F(VariableSplittingAnalyzerTest, SplitVariableNaming)
{
  auto expr = parser->parse("grad(grad(grad(u)))"); // 3rd order
  auto split_vars = analyzer->generateSplitVariables(expr);

  // Check naming convention
  expectSplitVariable(split_vars, "u_hess", "u", 2);  // 2nd order -> "hess"
  expectSplitVariable(split_vars, "u_d3", "u", 3);    // 3rd order -> "d3"

  // Test first order (should be "grad" but won't be generated here)
  auto expr_1st = parser->parse("u");
  analyzer->setMaxFEOrder(0); // Force splitting at 1st order
  auto split_vars_1st = analyzer->generateSplitVariables(expr_1st);
  analyzer->setMaxFEOrder(1); // Reset
}