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
#include "WeakFormGenerator.h"

#include <memory>
#include <string>

using namespace moose::automatic_weak_form;

/**
 * Unit tests for boundary term extraction from variational derivatives
 *
 * MATHEMATICAL BACKGROUND:
 * =======================
 * When we compute weak forms from energy functionals, integration by parts (IBP)
 * produces boundary integrals that represent natural boundary conditions.
 *
 * For an energy F[u] = ∫ f(u, ∇u, ∇²u, ...) dx, the variational derivative gives
 * coefficients C^k = ∂f/∂(∇^k u) at each derivative order k.
 *
 * After IBP, we get boundary terms: ∫_∂Ω (C^k · n) v ds
 *
 * These tests verify that we correctly extract these boundary terms.
 */
class BoundaryTermExtractionTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    parser = std::make_unique<StringExpressionParser>(3);
    generator = std::make_unique<WeakFormGenerator>(3);

    // Define test variables
    parser->defineVariable("u");
    parser->defineVariable("c");
    parser->defineVariable("T");

    // Define test parameters
    parser->setParameter("k", 1.0);
    parser->setParameter("kappa", 0.01);
    parser->setParameter("lambda", 0.001);
  }

  std::unique_ptr<StringExpressionParser> parser;
  std::unique_ptr<WeakFormGenerator> generator;
};

// Test 1: Simple diffusion energy - should produce Neumann BC
TEST_F(BoundaryTermExtractionTest, SimpleDiffusionEnergy)
{
  // Energy: F = ∫ ½k|∇u|² dx
  // This is the prototypical diffusion/Laplace problem
  auto energy = parser->parse("0.5 * k * dot(grad(u), grad(u))");
  ASSERT_NE(energy, nullptr);

  // Compute contributions
  auto contributions = generator->computeContributions(energy, "u");

  // Should have exactly one boundary term (from first-order gradient)
  ASSERT_EQ(contributions.boundary_terms.size(), 1);

  const auto & bt = contributions.boundary_terms[0];

  // Check basic properties
  EXPECT_EQ(bt.variable_name, "u");
  EXPECT_EQ(bt.field_derivative_order, 1);  // grad(u) appears in energy
  EXPECT_EQ(bt.test_derivative_order, 0);   // BC multiplies test function v, not ∇v
  EXPECT_EQ(bt.sign, -1);                   // First-order term gets negative sign

  // The coefficient should be k*grad(u) (the flux)
  // After contraction with normal n, boundary term is: -k(∇u·n)v
  EXPECT_NE(bt.coefficient, nullptr);

  // Verify it's related to the gradient
  std::string coeff_str = bt.coefficient->toString();
  EXPECT_TRUE(coeff_str.find("grad(u)") != std::string::npos ||
              coeff_str.find("k") != std::string::npos)
      << "Expected coefficient to involve k and grad(u), got: " << coeff_str;

  // Check description is informative
  EXPECT_TRUE(bt.description.find("Neumann") != std::string::npos ||
              bt.description.find("flux") != std::string::npos)
      << "Expected description to mention Neumann or flux, got: " << bt.description;
}

// Test 2: Fourth-order bending energy - should produce moment and shear BCs
TEST_F(BoundaryTermExtractionTest, BendingEnergy)
{
  // Energy: F = ∫ ½λ|∇²u|² dx
  // This is a fourth-order problem (thin plate bending)
  auto energy = parser->parse("0.5 * lambda * laplacian(u) * laplacian(u)");
  ASSERT_NE(energy, nullptr);

  auto contributions = generator->computeContributions(energy, "u");

  // Should have exactly one boundary term entry for the second-order coefficient
  // (In full theory, this expands to TWO physical BCs: moment and shear)
  ASSERT_EQ(contributions.boundary_terms.size(), 1);

  const auto & bt = contributions.boundary_terms[0];

  EXPECT_EQ(bt.variable_name, "u");
  EXPECT_EQ(bt.field_derivative_order, 2);  // laplacian(u) ~ ∇²u appears
  EXPECT_EQ(bt.test_derivative_order, 1);   // BC involves ∇v
  EXPECT_EQ(bt.sign, 1);                    // Second-order term gets positive sign

  // Check description mentions moment/curvature
  EXPECT_TRUE(bt.description.find("Moment") != std::string::npos ||
              bt.description.find("curvature") != std::string::npos)
      << "Expected description to mention moment or curvature, got: " << bt.description;
}

// Test 3: Cahn-Hilliard energy - gradient term only
TEST_F(BoundaryTermExtractionTest, CahnHilliardGradientEnergy)
{
  // Energy: F = ∫ [W(c) + ½κ|∇c|²] dx
  // Focus on the gradient term which produces a flux BC
  auto energy = parser->parse("0.5 * kappa * dot(grad(c), grad(c))");
  ASSERT_NE(energy, nullptr);

  auto contributions = generator->computeContributions(energy, "c");

  ASSERT_EQ(contributions.boundary_terms.size(), 1);

  const auto & bt = contributions.boundary_terms[0];

  EXPECT_EQ(bt.variable_name, "c");
  EXPECT_EQ(bt.field_derivative_order, 1);
  EXPECT_EQ(bt.test_derivative_order, 0);
  EXPECT_EQ(bt.sign, -1);

  // The flux should involve kappa and grad(c)
  std::string coeff_str = bt.coefficient->toString();
  EXPECT_TRUE(coeff_str.find("grad(c)") != std::string::npos)
      << "Expected coefficient to involve grad(c), got: " << coeff_str;
}

// Test 4: Pure bulk energy - no boundary terms
TEST_F(BoundaryTermExtractionTest, BulkEnergyNoBoundaryTerms)
{
  // Energy: F = ∫ u² dx
  // No spatial derivatives → no boundary terms
  auto energy = parser->parse("u * u");
  ASSERT_NE(energy, nullptr);

  auto contributions = generator->computeContributions(energy, "u");

  // Should have zero order term but NO boundary terms
  EXPECT_NE(contributions.c0_term, nullptr);
  EXPECT_EQ(contributions.boundary_terms.size(), 0)
      << "Pure bulk energy should not produce boundary terms";
}

// Test 5: Mixed energy - bulk + gradient
TEST_F(BoundaryTermExtractionTest, MixedBulkAndGradientEnergy)
{
  // Energy: F = ∫ [u² + ½k|∇u|²] dx
  // Should have boundary term only from gradient part
  auto energy = parser->parse("u*u + 0.5*k*dot(grad(u), grad(u))");
  ASSERT_NE(energy, nullptr);

  auto contributions = generator->computeContributions(energy, "u");

  // Should have both C^0 (from u²) and C^1 (from |∇u|²)
  EXPECT_NE(contributions.c0_term, nullptr);
  EXPECT_NE(contributions.c1_term, nullptr);

  // But only ONE boundary term (from C^1)
  ASSERT_EQ(contributions.boundary_terms.size(), 1);

  const auto & bt = contributions.boundary_terms[0];
  EXPECT_EQ(bt.field_derivative_order, 1);
}

// Test 6: Verify extractBoundaryTerms() directly
TEST_F(BoundaryTermExtractionTest, DirectExtractionMethod)
{
  // Create a simple differential manually
  Differential diff;

  // Add first-order coefficient: C¹ = grad(u)
  auto u = parser->parse("u");
  diff.coefficients[1] = parser->parse("grad(u)");

  // Extract boundary terms
  auto boundary_expr = generator->extractBoundaryTerms(diff, "u");

  ASSERT_NE(boundary_expr, nullptr);

  // Should involve normal vector 'n' and grad(u)
  std::string boundary_str = boundary_expr->toString();
  EXPECT_TRUE(boundary_str.find("n") != std::string::npos)
      << "Boundary term should involve normal vector n, got: " << boundary_str;
  EXPECT_TRUE(boundary_str.find("grad(u)") != std::string::npos)
      << "Boundary term should involve grad(u), got: " << boundary_str;
}

// Test 7: Multiple variables - boundary terms for each
TEST_F(BoundaryTermExtractionTest, MultipleVariables)
{
  // Energy: F = ∫ [½|∇u|² + ½|∇c|²] dx
  // Should get boundary terms for both u and c when we differentiate w.r.t. each

  auto energy = parser->parse("0.5*dot(grad(u), grad(u)) + 0.5*dot(grad(c), grad(c))");
  ASSERT_NE(energy, nullptr);

  // Differentiate w.r.t. u
  auto contrib_u = generator->computeContributions(energy, "u");
  ASSERT_EQ(contrib_u.boundary_terms.size(), 1);
  EXPECT_EQ(contrib_u.boundary_terms[0].variable_name, "u");

  // Differentiate w.r.t. c
  auto contrib_c = generator->computeContributions(energy, "c");
  ASSERT_EQ(contrib_c.boundary_terms.size(), 1);
  EXPECT_EQ(contrib_c.boundary_terms[0].variable_name, "c");
}

// Test 8: Verify max_order is correctly set
TEST_F(BoundaryTermExtractionTest, MaxOrderTracking)
{
  // First-order energy
  {
    auto energy = parser->parse("0.5*dot(grad(u), grad(u))");
    auto contributions = generator->computeContributions(energy, "u");
    EXPECT_EQ(contributions.max_order, 1);
    EXPECT_EQ(contributions.boundary_terms.size(), 1);
  }

  // Second-order energy
  {
    auto energy = parser->parse("0.5*laplacian(u)*laplacian(u)");
    auto contributions = generator->computeContributions(energy, "u");
    EXPECT_EQ(contributions.max_order, 2);
    EXPECT_EQ(contributions.boundary_terms.size(), 1);
  }

  // Zeroth-order energy
  {
    auto energy = parser->parse("u*u");
    auto contributions = generator->computeContributions(energy, "u");
    EXPECT_EQ(contributions.max_order, 0);
    EXPECT_EQ(contributions.boundary_terms.size(), 0);
  }
}
