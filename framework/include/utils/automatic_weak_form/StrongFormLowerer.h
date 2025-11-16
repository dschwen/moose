#pragma once

#include "MooseAST.h"
#include <utility>
#include <functional>
#include <string>

namespace moose
{
namespace automatic_weak_form
{

// Strong-form → weak-form integration-by-parts (IBP) lowerer
//
// Given a strong-form RHS expression g(u, grad(u), ...), this utility
// derives weak-form contributions (c0, c1) such that the contribution to
// the residual is:
//   R = ∫ [ c0 * v + c1 · grad(v) ] dx   (boundary terms ignored)
// for the default test function v of the owning variable.
//
// Notes/Limitations (initial version):
// - Handles patterns: div(V), a*div(V), laplacian(u), a*laplacian(u)
// - For a*div(V) and a*laplacian(u), product rule introduces V·grad(a) terms.
//   We include these terms only if 'a' does not depend on field variables
//   (i.e., constant/parameter). Otherwise we skip that part and emit a warning.
// - All other terms g are treated as bulk: contribute c0 += -g.
// - Boundary terms from IBP are ignored here (natural BCs not generated).
//
class StrongFormLowerer
{
public:
  explicit StrongFormLowerer(unsigned int dim) : _dim(dim) {}
  
  // Enable or disable verbose tracing
  void setVerbose(bool v) { _verbose = v; }
  
  // Optional tracer callback for logging derivation steps
  // If not set, messages are printed via Moose::out when _verbose is true
  void setTracer(std::function<void(const std::string &)> cb) { _tracer = std::move(cb); }

  struct Lowered
  {
    NodePtr c0; // scalar
    NodePtr c1; // vector (dim)
  };

  Lowered lowerToWeak(const NodePtr & expr);

private:
  unsigned int _dim;
  bool _verbose = false;
  std::function<void(const std::string &)> _tracer;

  // Recursively lower expression into (c0, c1)
  Lowered lower(const NodePtr & node);
  
  void trace(const std::string & msg) const;

  // Helpers
  bool isZero(const NodePtr & n) const;
  NodePtr addSafe(const NodePtr & a, const NodePtr & b) const;
  NodePtr subSafe(const NodePtr & a, const NodePtr & b) const;
  NodePtr mulSafe(const NodePtr & a, const NodePtr & b) const;
  NodePtr neg(const NodePtr & a) const;

  bool dependsOnField(const NodePtr & n) const;
};

}
}
