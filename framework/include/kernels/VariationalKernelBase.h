#pragma once

#include "GenericKernel.h"
#include "MooseAST.h"
#include "MooseExpressionBuilder.h"
#include "ExpressionEvaluator.h"

namespace moose
{
namespace automatic_weak_form
{

// Forward declaration of the template
template <bool is_ad>
class VariationalKernelBaseTempl;

// Typedefs for convenience
typedef VariationalKernelBaseTempl<false> VariationalKernelBase;
typedef VariationalKernelBaseTempl<true> ADVariationalKernelBase;

template <bool is_ad>
class VariationalKernelBaseTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  VariationalKernelBaseTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  void initializeExpression();
  void precomputeDerivativesLowered();

  MooseValue evaluateExpression(const NodePtr & expr);
  MooseValue evaluateAtQP(const NodePtr & expr, unsigned int qp);

  // AD-compatible evaluation (used when is_ad=true)
  GenericReal<is_ad> evaluateExpressionAD(const NodePtr & expr, unsigned int qp);

  void initializeEvaluator();

  void updateVariableValues(unsigned int qp);

  void updateEvaluatorAtQP(unsigned int qp);

  NodePtr differentiateWithRespectToField(const NodePtr & expr, const std::string & var_name);

  NodePtr differentiateWithRespectToGradient(const NodePtr & expr, const std::string & var_name);

  bool containsGradient(const NodePtr & expr, const std::string & var_name);

  void validateDifferentialOperators(const NodePtr & expr) const;

protected:
  // Expression parsing/evaluation
  std::unique_ptr<MooseExpressionBuilder> _builder;
  std::unique_ptr<ExpressionEvaluator> _evaluator;

  // Coupled variable data
  std::map<std::string, const VariableValue *> _coupled_values;
  std::map<std::string, const VariableGradient *> _coupled_gradients;
  std::map<std::string, const VariableSecond *> _coupled_seconds;
  std::map<std::string, unsigned int> _coupled_var_nums;
  std::map<std::string, MooseValue> _variable_cache;

  unsigned int _max_derivative_order;
  bool _compute_jacobian_numerically;
  Real _fd_eps;

  std::map<std::string, Real> _parameters;
  std::vector<std::string> _coupled_variable_names;

  bool _use_automatic_differentiation;
  bool _enable_variable_splitting;
  unsigned int _fe_order;

  std::map<std::string, NodePtr> _split_variables;

  void clearCache() { _variable_cache.clear(); }

  bool _is_strong_form;

  // Lowered strong-form weak contributions (optional)
  bool _use_lowered_weak_form = false;
  NodePtr _c0_expr; // scalar
  NodePtr _c1_expr; // vector
  NodePtr _residual_expr; // scalar residual AST: c0*test + c1·grad(test)

  // Cached symbolic derivatives for lowered weak form (per variable name)
  std::map<std::string, NodePtr> _dc0_du;
  std::map<std::string, NodePtr> _dc0_dgradu;
  std::map<std::string, NodePtr> _dc1_du;
  std::map<std::string, NodePtr> _dc1_dgradu;

  // Scalar Jacobian ASTs for non-AD evaluation: map wrt-var -> expression
  std::map<std::string, NodePtr> _jac_expr_map;

  // Using statements for base class members
  usingGenericKernelMembers;
};

}
}
