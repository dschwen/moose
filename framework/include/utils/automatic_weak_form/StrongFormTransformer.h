#pragma once

#include "MooseAST.h"
#include <memory>

namespace moose
{
namespace automatic_weak_form
{

/**
 * Transforms strong form expressions to their weak form equivalents
 * by applying integration by parts to differential operators.
 *
 * For example:
 *   laplacian(u) -> needs integration by parts
 *   After transformation: represents -div(grad(u)) which becomes +grad(u)·grad(ψ) in weak form
 */
class StrongFormTransformer
{
public:
  StrongFormTransformer(unsigned int dim);

  /**
   * Transform a strong form expression for use in weak form residual
   * @param expr The strong form RHS expression
   * @param is_strong_form If true, apply IBP transformations
   * @return Transformed expression suitable for weak form evaluation
   */
  NodePtr transform(const NodePtr & expr, bool is_strong_form = true);

private:
  /**
   * Recursively transform nodes, applying IBP where needed
   */
  NodePtr transformNode(const NodePtr & node);

  /**
   * Transform laplacian(u) -> -div(grad(u))
   * In weak form, this becomes: -∫ laplacian(u)*ψ dx = ∫ grad(u)·grad(ψ) dx
   * We mark this for special handling in the kernel
   */
  NodePtr transformLaplacian(const NodePtr & operand);

  /**
   * Check if this node contains a laplacian that needs transformation
   */
  bool containsLaplacian(const NodePtr & node);

  unsigned int _dim;
};

} // namespace automatic_weak_form
} // namespace moose
