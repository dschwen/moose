#include "VariationalKernelBase.h"
#include "MooseError.h"
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"
#include <unordered_map>
#include <functional>

namespace moose
{
namespace automatic_weak_form
{

// Template instantiations and registrations at end of file

template <bool is_ad>
InputParameters
VariationalKernelBaseTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();

  params.addClassDescription("Base class for variational derivative kernels that automatically "
                              "generate weak forms from energy functionals");

  // Add a generic coupled variable parameter that can handle multiple variables
  params.addCoupledVar("coupled_vars", "List of coupled variables from the energy expression");
  
  params.addParam<std::map<std::string, Real>>("parameters",
                                                 "Named parameters for the energy functional");

  params.addParam<bool>("use_automatic_differentiation", true,
                         "Use automatic differentiation for Jacobian");

  params.addParam<bool>("compute_jacobian_numerically", false,
                         "Compute Jacobian using finite differences");

  params.addParam<Real>("fd_eps", 1e-8,
                         "Finite difference epsilon for numerical Jacobian");

  params.addParam<bool>("enable_variable_splitting", true,
                         "Enable automatic variable splitting for higher-order derivatives");

  params.addParam<unsigned int>("fe_order", 1,
                                  "Finite element order");

  params.addParam<bool>("has_time_derivatives", false,
                         "Whether the problem includes time derivatives");

  params.addParam<bool>("is_strong_form", false,
                        "If true, expression is a strong form residual (use directly); "
                        "if false, expression is an energy functional (compute variational derivative)");

  // Optional: accept pre-lowered weak-form pieces (c0, c1)
  params.addParam<bool>("use_lowered_weak_form", false,
                        "If true, use provided c0/c1 expressions to assemble weak form");
  params.addParam<std::string>("c0_expression", "",
                                "Scalar c0(u,grad(u),...) used as c0*test in weak residual");
  params.addParam<std::string>("c1_expression", "",
                                "Vector c1(u,grad(u),...) used as c1·grad(test) in weak residual");

  return params;
}

template <bool is_ad>
VariationalKernelBaseTempl<is_ad>::VariationalKernelBaseTempl(const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters),
    _compute_jacobian_numerically(this->template getParam<bool>("compute_jacobian_numerically")),
    _fd_eps(this->template getParam<Real>("fd_eps")),
    _parameters(this->isParamValid("parameters") ? this->template getParam<std::map<std::string, Real>>("parameters") : std::map<std::string, Real>()),
    _use_automatic_differentiation(this->template getParam<bool>("use_automatic_differentiation")),
    _enable_variable_splitting(this->template getParam<bool>("enable_variable_splitting")),
    _fe_order(this->template getParam<unsigned int>("fe_order")),
    _is_strong_form(this->template getParam<bool>("is_strong_form"))
{
  _builder = std::make_unique<MooseExpressionBuilder>(this->_mesh.dimension());
  _evaluator = std::make_unique<ExpressionEvaluator>(this->_mesh.dimension());


  // Get the list of coupled variables from the generic parameter
  if (this->isCoupled("coupled_vars"))
  {
    unsigned int n_coupled = this->coupledComponents("coupled_vars");
    _coupled_variable_names.clear();

    for (unsigned int i = 0; i < n_coupled; ++i)
    {
      auto var_name = this->getFieldVar("coupled_vars", i)->name();
      _coupled_variable_names.push_back(var_name);

      // Store the coupled values and gradients
      _coupled_values[var_name] = &this->coupledValue("coupled_vars", i);
      _coupled_gradients[var_name] = &this->coupledGradient("coupled_vars", i);

      // For strong forms or higher-order elements, store second derivatives
      if (_is_strong_form || _fe_order >= 2)
        _coupled_seconds[var_name] = &this->coupledSecond("coupled_vars", i);
    }
  }

  initializeExpression();
  initializeEvaluator();
}

template <bool is_ad>
void
VariationalKernelBaseTempl<is_ad>::initializeEvaluator()
{
  // Register the primary variable
  _evaluator->registerVariable(_var.name(), _var.number());

  // Register all coupled variables
  unsigned int var_counter = _var.number() + 1;
  for (const auto & var_name : _coupled_variable_names)
  {
    // If we don't already have a mapping, create one
    if (_coupled_var_nums.find(var_name) == _coupled_var_nums.end())
      _coupled_var_nums[var_name] = var_counter++;

    _evaluator->registerVariable(var_name, _coupled_var_nums[var_name]);
  }

  // Register all parameters
  for (const auto & [param_name, param_value] : _parameters)
    _evaluator->setParameter(param_name, param_value);
}

template <bool is_ad>
void
VariationalKernelBaseTempl<is_ad>::updateEvaluatorAtQP(unsigned int qp)
{
  // This method is only called in non-AD path for symbolic evaluation
  if constexpr (is_ad)
  {
    this->mooseError("updateEvaluatorAtQP should not be called with AD");
    return;
  }
  else
  {
  // Update primary variable
  _evaluator->setFieldValue(_var.number(), this->_u[qp]);
  _evaluator->setFieldGradient(_var.number(), this->_grad_u[qp]);
  // Set test function values for primary variable
  _evaluator->setTestFunction(this->_var.number(), this->_test[this->_i][qp]);
  _evaluator->setTestGradient(this->_var.number(), this->_grad_test[this->_i][qp]);

  // For strong forms with Laplacian, we need second derivatives.
  // Skip when using lowered weak form (c0/c1 only needs values/gradients).
  if (_is_strong_form && !_use_lowered_weak_form)
  {
    // Access second derivatives through the variable interface
    const auto & second_u = _var.secondSln();
    _evaluator->setFieldHessian(_var.number(), second_u[qp]);
  }

  // Update coupled variables
  for (const auto & var_name : _coupled_variable_names)
  {
    unsigned int var_num = _coupled_var_nums[var_name];

    if (_coupled_values.count(var_name))
      _evaluator->setFieldValue(var_num, (*_coupled_values[var_name])[qp]);

    if (_coupled_gradients.count(var_name))
      _evaluator->setFieldGradient(var_num, (*_coupled_gradients[var_name])[qp]);

    // For strong forms (non-lowered) or higher-order elements, set second derivatives when available
    if (((_is_strong_form && !_use_lowered_weak_form) || _fe_order >= 2) && _coupled_seconds.count(var_name))
      _evaluator->setFieldHessian(var_num, (*_coupled_seconds[var_name])[qp]);
  }
  }
}

template <bool is_ad>
void
VariationalKernelBaseTempl<is_ad>::initializeExpression()
{
  // If using lowered weak form, parse c0/c1 expressions now
  if (this->isParamValid("use_lowered_weak_form") && this->template getParam<bool>("use_lowered_weak_form"))
  {
    _use_lowered_weak_form = true;

    const auto & c0s = this->template getParam<std::string>("c0_expression");
    const auto & c1s = this->template getParam<std::string>("c1_expression");

    if (!c0s.empty())
      _c0_expr = _builder->parseExpression(c0s, _parameters);
    if (!c1s.empty())
      _c1_expr = _builder->parseExpression(c1s, _parameters);

    // Precompute symbolic derivatives for Jacobian (huge runtime savings)
    precomputeDerivativesLowered();

    // For non-AD, assemble a scalar residual AST: c0*test + c1·grad(test)
    if constexpr (!is_ad)
    {
      NodePtr term0 = nullptr;
      NodePtr term1 = nullptr;
      if (_c0_expr)
      {
        auto t = testFunction(this->_var.name(), false);
        term0 = multiply(_c0_expr, t);
      }
      if (_c1_expr)
      {
        auto gt = testFunction(this->_var.name(), true);
        term1 = dot(_c1_expr, gt);
      }
      if (term0 && term1)
        _residual_expr = add(term0, term1);
      else if (term0)
        _residual_expr = term0;
      else if (term1)
        _residual_expr = term1;

      // Also assemble scalar Jacobian ASTs per variable
      auto build_jac_for = [&](const std::string & wrt) -> NodePtr {
        NodePtr sum = nullptr;
        // t0: (∂c0/∂u) * phi_w * test_u
        if (auto it = _dc0_du.find(wrt); it != _dc0_du.end() && it->second)
        {
          NodePtr t0 = multiply(it->second, multiply(shapeFunction(wrt, false), testFunction(this->_var.name(), false)));
          sum = sum ? add(sum, t0) : t0;
        }
        // t1: dot(∂c0/∂∇u, grad_phi_w) * test_u
        if (auto it = _dc0_dgradu.find(wrt); it != _dc0_dgradu.end() && it->second)
        {
          NodePtr t1 = multiply(dot(it->second, shapeFunction(wrt, true)), testFunction(this->_var.name(), false));
          sum = sum ? add(sum, t1) : t1;
        }
        // t2: dot(∂c1/∂u, grad_test_u) * phi_w
        if (auto it = _dc1_du.find(wrt); it != _dc1_du.end() && it->second)
        {
          NodePtr t2 = multiply(dot(it->second, testFunction(this->_var.name(), true)), shapeFunction(wrt, false));
          sum = sum ? add(sum, t2) : t2;
        }
        // t3: (∂c1/∂∇u) : (grad_phi_w ⊗ grad_test_u)
        if (auto it = _dc1_dgradu.find(wrt); it != _dc1_dgradu.end() && it->second)
        {
          const auto & d = it->second;
          NodePtr grad_phi = shapeFunction(wrt, true);
          NodePtr grad_test = testFunction(this->_var.name(), true);
          NodePtr t3 = nullptr;
          if (d->isTensor())
          {
            t3 = contract(d, outer(grad_phi, grad_test));
          }
          else // assume scalar coefficient
          {
            t3 = multiply(d, dot(grad_phi, grad_test));
          }
          sum = sum ? add(sum, t3) : t3;
        }
        return sum;
      };

      // Build for primary variable and all coupled vars
      _jac_expr_map.clear();
      _jac_expr_map[this->_var.name()] = build_jac_for(this->_var.name());
      for (const auto & n : _coupled_variable_names)
        _jac_expr_map[n] = build_jac_for(n);
    }
  }
}

template <bool is_ad>
void
VariationalKernelBaseTempl<is_ad>::precomputeDerivativesLowered()
{
  if (!_use_lowered_weak_form)
    return;

  // Collect all variable names involved: primary + coupled
  std::vector<std::string> vars;
  vars.push_back(this->_var.name());
  for (const auto & n : _coupled_variable_names)
    vars.push_back(n);

  for (const auto & name : vars)
  {
    if (_c0_expr)
    {
      _dc0_du[name] = differentiateWithRespectToField(_c0_expr, name);
      _dc0_dgradu[name] = differentiateWithRespectToGradient(_c0_expr, name);
    }
    if (_c1_expr)
    {
      _dc1_du[name] = differentiateWithRespectToField(_c1_expr, name);
      _dc1_dgradu[name] = differentiateWithRespectToGradient(_c1_expr, name);
    }
  }
}

template <bool is_ad>
void
VariationalKernelBaseTempl<is_ad>::updateVariableValues(unsigned int qp)
{
  // This method is only called in non-AD path for symbolic evaluation
  if constexpr (is_ad)
  {
    this->mooseError("updateVariableValues should not be called with AD");
  }
  else
  {
  _variable_cache.clear();

  _variable_cache[_var.name()] = MooseValue(this->_u[qp]);

  RealVectorValue grad_u(this->_grad_u[qp]);
  _variable_cache[_var.name() + "_grad"] = MooseValue(grad_u, this->_mesh.dimension());

  for (const auto & [name, value_ptr] : _coupled_values)
  {
    _variable_cache[name] = MooseValue((*value_ptr)[qp]);

    if (_coupled_gradients.count(name))
    {
      RealVectorValue grad((*_coupled_gradients[name])[qp]);
      _variable_cache[name + "_grad"] = MooseValue(grad, this->_mesh.dimension());
    }
  }
  }
}

template <bool is_ad>
GenericReal<is_ad>
VariationalKernelBaseTempl<is_ad>::computeQpResidual()
{
  // Lowered weak form (assemble c0*test + c1·grad(test))
  if (_use_lowered_weak_form)
  {
    if constexpr (is_ad)
    {
      // AD path: evaluate c0 (scalar) with AD and c1·grad(test) via pattern-matched dot
      GenericReal<is_ad> R = 0.0;
      if (_c0_expr)
        R += evaluateExpressionAD(_c0_expr, this->_qp) * this->_test[this->_i][this->_qp];
      if (_c1_expr)
      {
        // Compute dot(c1(u,grad(u),...), grad(test)) directly with AD
        std::function<GenericReal<is_ad>(const NodePtr &)> dot_val;
        dot_val = [this, &dot_val](const NodePtr & expr) -> GenericReal<is_ad>
        {
          // Constant 0: treat as zero contribution
          if (expr->type() == NodeType::Constant)
          {
            auto c = std::static_pointer_cast<ConstantNode>(expr);
            if (c->value().isScalar() && c->value().asScalar() == 0.0)
              return GenericReal<is_ad>(0.0);
            this->mooseError("Non-zero scalar constant used as c1 in AD lowered path: ", expr->toString());
          }
          // Handle sums of vector contributions: dot(sum, gtest) = sum dot(...)
          if (expr->type() == NodeType::Add)
          {
            auto bin = std::static_pointer_cast<BinaryOpNode>(expr);
            return dot_val(bin->left()) + dot_val(bin->right());
          }
          if (expr->type() == NodeType::Subtract)
          {
            auto bin = std::static_pointer_cast<BinaryOpNode>(expr);
            return dot_val(bin->left()) - dot_val(bin->right());
          }
          if (expr->type() == NodeType::Negate)
          {
            auto un = std::static_pointer_cast<UnaryOpNode>(expr);
            return -dot_val(un->operand());
          }
          if (expr->type() == NodeType::Gradient)
          {
            // grad(var) · grad(test)
            auto unary = std::static_pointer_cast<UnaryOpNode>(expr);
            auto op = unary->operand();
            std::string name;
            if (op->type() == NodeType::Variable)
              name = std::static_pointer_cast<VariableNode>(op)->name();
            else if (op->type() == NodeType::FieldVariable)
              name = std::static_pointer_cast<FieldVariableNode>(op)->name();
            else
              this->mooseError("Unsupported operand to Gradient in AD lowered path: ", op->toString());

            const auto & gtest = this->_grad_test[this->_i][this->_qp];
            if (name == this->_var.name())
              return this->_grad_u[this->_qp] * gtest;
            else if (_coupled_gradients.count(name))
              return (*_coupled_gradients[name])[this->_qp] * gtest;
            else
              this->mooseError("Unknown variable in Gradient for AD lowered path: ", name);
          }
          if (expr->type() == NodeType::Multiply)
          {
            auto bin = std::static_pointer_cast<BinaryOpNode>(expr);
            // Look for scalar * grad(var) or grad(var) * scalar
            if (bin->left()->type() == NodeType::Gradient)
            {
              auto coeff = evaluateExpressionAD(bin->right(), this->_qp);
              return coeff * dot_val(bin->left());
            }
            if (bin->right()->type() == NodeType::Gradient)
            {
              auto coeff = evaluateExpressionAD(bin->left(), this->_qp);
              return coeff * dot_val(bin->right());
            }
            // Fallback not supported for complex vector expressions in AD path
            this->mooseError("Unsupported c1 structure for AD lowered path: ", expr->toString());
          }
          // Not a vector expression we support for dot-product evaluation
          this->mooseError("Unsupported node for c1·grad(test) in AD lowered path: ", expr->toString());
        };

        R += dot_val(_c1_expr);
      }
      return R;
    }
    else
    {
      clearCache();
      updateVariableValues(_qp);
      if (_residual_expr)
      {
        MooseValue v = evaluateExpression(_residual_expr);
        if (v.isScalar())
          return v.asScalar();
      }
      // Fallback to explicit split if residual AST missing
      Real residual = 0.0;
      if (_c0_expr)
      {
        MooseValue v0 = evaluateExpression(_c0_expr);
        if (v0.isScalar())
          residual += v0.asScalar() * this->_test[this->_i][this->_qp];
      }
      if (_c1_expr)
      {
        MooseValue v1 = evaluateExpression(_c1_expr);
        if (v1.isVector())
          residual += v1.asVector() * this->_grad_test[this->_i][this->_qp];
      }
      return residual;
    }
  }
  this->mooseError("Lowered weak form not provided; only strong_forms with IBP lowering are supported.");
  return 0.0;
}

template <bool is_ad>
Real
VariationalKernelBaseTempl<is_ad>::computeQpJacobian()
{
  // When using AD (is_ad=true), MOOSE computes Jacobian automatically via dual numbers
  // This method is not called, but must still compile
  if constexpr (is_ad)
  {
    // This shouldn't be called when is_ad=true
    this->mooseError("computeQpJacobian should not be called when using AD");
    return 0.0;
  }
  else  // Only compile non-AD path
  {
  // Lowered weak form Jacobian (non-AD)
  if (_use_lowered_weak_form)
  {
    clearCache();
    updateVariableValues(_qp);
    // Provide shape for primary variable
    _evaluator->setShapeFunction(this->_var.number(), this->_phi[this->_j][this->_qp]);
    _evaluator->setShapeGradient(this->_var.number(), this->_grad_phi[this->_j][this->_qp]);

    auto it = _jac_expr_map.find(this->_var.name());
    if (it != _jac_expr_map.end() && it->second)
    {
      MooseValue v = evaluateExpression(it->second);
      if (v.isScalar())
        return v.asScalar();
    }
    return 0.0;
  }

  if (_compute_jacobian_numerically)
  {
    // Numerical Jacobian computation disabled - _u is const in MOOSE
    // Use automatic differentiation or analytical Jacobian instead
    mooseWarning("Numerical Jacobian computation not available, using base class implementation");
    return GenericKernel<is_ad>::computeQpJacobian();
  }
  // Lowered weak form Jacobian (non-AD)
  if (_use_lowered_weak_form)
  {
    clearCache();
    updateVariableValues(_qp);
    _evaluator->setShapeFunction(this->_var.number(), this->_phi[this->_j][this->_qp]);
    _evaluator->setShapeGradient(this->_var.number(), this->_grad_phi[this->_j][this->_qp]);
    auto it = _jac_expr_map.find(this->_var.name());
    if (it != _jac_expr_map.end() && it->second)
    {
      MooseValue v = evaluateExpression(it->second);
      if (v.isScalar())
        return v.asScalar();
    }
    return 0.0;
  }

  this->mooseError("Lowered weak form not provided; only strong_forms with IBP lowering are supported.");
  return 0.0;
  }  // end else (!is_ad)
}

template <bool is_ad>
Real
VariationalKernelBaseTempl<is_ad>::computeQpOffDiagJacobian(unsigned int jvar)
{
  // When using AD (is_ad=true), MOOSE computes Jacobian automatically via dual numbers
  // This method is not called, but must still compile
  if constexpr (is_ad)
  {
    this->mooseError("computeQpOffDiagJacobian should not be called when using AD");
    return 0.0;
  }
  else
  {
    // Only supported for lowered weak form
    if (!_use_lowered_weak_form)
      return 0.0;

    // Resolve the coupled variable name for jvar
    std::string coupled_var_name;
    for (const auto & [name, var_num] : _coupled_var_nums)
    {
      if (var_num == jvar)
      {
        coupled_var_name = name;
        break;
      }
    }
    if (coupled_var_name.empty())
      return 0.0;

    clearCache();
    updateVariableValues(this->_qp);

    // Determine var_num for shape function entries
    unsigned int var_num = 0;
    if (coupled_var_name == this->_var.name())
      var_num = this->_var.number();
    else if (auto itvn = _coupled_var_nums.find(coupled_var_name); itvn != _coupled_var_nums.end())
      var_num = itvn->second;
    else
      return 0.0;

    _evaluator->setShapeFunction(var_num, this->_phi[this->_j][this->_qp]);
    _evaluator->setShapeGradient(var_num, this->_grad_phi[this->_j][this->_qp]);

    auto itje = _jac_expr_map.find(coupled_var_name);
    if (itje != _jac_expr_map.end() && itje->second)
    {
      MooseValue v = evaluateExpression(itje->second);
      if (v.isScalar())
        return v.asScalar();
    }
    return 0.0;
  }  // end else (!is_ad)
}

// Removed energy-functional contribution helpers; only lowered strong-form is supported

template <bool is_ad>
MooseValue
VariationalKernelBaseTempl<is_ad>::evaluateExpression(const NodePtr & expr)
{
  return evaluateAtQP(expr, _qp);
}

template <bool is_ad>
MooseValue
VariationalKernelBaseTempl<is_ad>::evaluateAtQP(const NodePtr & expr, unsigned int qp)
{
  if (!expr)
    return MooseValue(0.0);

  // Validate that expression doesn't contain differential operators (only for energy functionals)
  // For strong forms, differential operators like laplacian() are allowed
  // For energy functionals after integration by parts and variable splitting, we should only see:
  // - Field variables (u, coupled_vars)
  // - Gradients of field variables (grad(u), grad(coupled_var))
  // - Parameters and constants
  // We should NOT see:
  // - div(field_var) - should be eliminated by integration by parts
  // - laplacian(field_var) - should be eliminated by variable splitting
  // - curl(field_var) - should be eliminated by variable splitting
  if (!_is_strong_form)
    validateDifferentialOperators(expr);

  // Update the evaluator with values at this quadrature point
  updateEvaluatorAtQP(qp);

  // Use ExpressionEvaluator to evaluate the expression
  // This handles all operations: arithmetic, vector ops, tensor ops, functions, etc.
  try {
    // ExpressionEvaluator has public methods for different return types
    // For now, try to evaluate as scalar, vector, or tensor based on the expression shape
    if (!expr->shape().index())  // ScalarShape
    {
      Real result = _evaluator->evaluate(expr);
      return MooseValue(result);
    }
    else if (expr->shape().index() == 1)  // VectorShape
    {
      RealVectorValue result = _evaluator->evaluateVector(expr);
      return MooseValue(result, this->_mesh.dimension());
    }
    else if (expr->shape().index() == 2)  // TensorShape
    {
      RankTwoTensor result = _evaluator->evaluateTensor(expr);
      return MooseValue(result, this->_mesh.dimension());
    }
    else
    {
      mooseError("Unsupported expression shape index: ", expr->shape().index());
    }
  }
  catch (const std::exception & e) {
    // If evaluation fails, provide helpful error message
    mooseError("Failed to evaluate expression at QP ", qp, ":\n",
               "Expression: ", expr->toString(), "\n",
               "Error: ", e.what());
  }
}

template <bool is_ad>
void
VariationalKernelBaseTempl<is_ad>::validateDifferentialOperators(const NodePtr & expr) const
{
  if (!expr)
    return;

  // Check this node
  switch (expr->type())
  {
    case NodeType::Divergence:
    {
      // Check if divergence is applied to a field variable (not allowed)
      auto unary = std::static_pointer_cast<UnaryOpNode>(expr);
      auto operand = unary->operand();

      if (operand->type() == NodeType::FieldVariable || operand->type() == NodeType::Variable)
      {
        mooseError("div(", operand->toString(), ") appeared in weak form coefficient.\n",
                   "Divergence of field variables should be eliminated by integration by parts.\n",
                   "This indicates the weak form generator did not properly apply integration by parts.");
      }

      // div(grad(u)) might appear and should be caught
      if (operand->type() == NodeType::Gradient)
      {
        auto grad_unary = std::static_pointer_cast<UnaryOpNode>(operand);
        auto grad_operand = grad_unary->operand();
        if (grad_operand->type() == NodeType::FieldVariable || grad_operand->type() == NodeType::Variable)
        {
          mooseError("div(grad(", grad_operand->toString(), ")) (laplacian) appeared in weak form coefficient.\n",
                     "This second-order operator should be eliminated by either:\n",
                     "  1. Variable splitting (introduce auxiliary variable q = grad(u))\n",
                     "  2. Using higher-order finite elements\n",
                     "Consider enabling 'enable_variable_splitting = true' in the input file.");
        }
      }
      break;
    }

    case NodeType::Laplacian:
    {
      // Laplacian should never appear on field variables after splitting
      auto unary = std::static_pointer_cast<UnaryOpNode>(expr);
      auto operand = unary->operand();
      if (operand->type() == NodeType::FieldVariable || operand->type() == NodeType::Variable)
      {
        mooseError("laplacian(", operand->toString(), ") appeared in weak form coefficient.\n",
                   "Second-order operators should be eliminated by variable splitting.\n",
                   "Enable 'enable_variable_splitting = true' and ensure max_fe_order is set appropriately.");
      }
      break;
    }

    case NodeType::Curl:
    {
      // Curl of field variables requires special handling
      auto unary = std::static_pointer_cast<UnaryOpNode>(expr);
      auto operand = unary->operand();
      if (operand->type() == NodeType::FieldVariable || operand->type() == NodeType::Variable)
      {
        mooseError("curl(", operand->toString(), ") appeared in weak form coefficient.\n",
                   "Curl of field variables should be handled via variable splitting or edge elements.\n",
                   "For H(curl) problems, use specialized finite element spaces.");
      }
      break;
    }

    default:
      break;
  }

  // Recursively validate children
  for (const auto & child : expr->children())
    validateDifferentialOperators(child);
}

// Rest of the original evaluateAtQP implementation removed - now using ExpressionEvaluator
// The old implementation can be found in git history if needed

template <bool is_ad>
NodePtr
VariationalKernelBaseTempl<is_ad>::differentiateWithRespectToField(const NodePtr & expr, const std::string & var_name)
{
  if (!expr)
    return nullptr;
    
  // Use the differentiation visitor to compute ∂expr/∂var
  DifferentiationVisitor dv(var_name);
  auto diff = dv.differentiate(expr);
  
  // The 0-th order coefficient is ∂expr/∂var
  if (diff.hasOrder(0))
    return diff.getCoefficient(0);
    
  return nullptr;
}

template <bool is_ad>
NodePtr
VariationalKernelBaseTempl<is_ad>::differentiateWithRespectToGradient(const NodePtr & expr, const std::string & var_name)
{
  if (!expr)
    return nullptr;
    
  // For expressions like C^1 = κ∇u, we need ∂C^1/∂(∇u)
  // This is more complex and depends on the structure of the expression
  
  // Handle different node types
  if (expr->type() == NodeType::Add)
  {
    // d(a + b)/d(grad u) = da/d(grad u) + db/d(grad u)
    auto binary = std::static_pointer_cast<BinaryOpNode>(expr);
    auto left_deriv = differentiateWithRespectToGradient(binary->left(), var_name);
    auto right_deriv = differentiateWithRespectToGradient(binary->right(), var_name);
    
    if (left_deriv && right_deriv)
      return add(left_deriv, right_deriv);
    else if (left_deriv)
      return left_deriv;
    else if (right_deriv)
      return right_deriv;
    else
      return nullptr;
  }
  else if (expr->type() == NodeType::Multiply)
  {
    // d(a * b)/d(grad u) = a * db/d(grad u) + da/d(grad u) * b
    // But if a is scalar and b is grad(u), then d(a * grad(u))/d(grad u) = a * I
    auto binary = std::static_pointer_cast<BinaryOpNode>(expr);
    auto left = binary->left();
    auto right = binary->right();
    
    // Check if one side is a scalar constant/expression and the other contains grad(u)
    bool left_has_grad = containsGradient(left, var_name);
    bool right_has_grad = containsGradient(right, var_name);
    
    if (!left_has_grad && right_has_grad)
    {
      // left is coefficient, right contains grad(u)
      auto right_deriv = differentiateWithRespectToGradient(right, var_name);
      if (right_deriv)
        return multiply(left, right_deriv);
    }
    else if (left_has_grad && !right_has_grad)
    {
      // right is coefficient, left contains grad(u)
      auto left_deriv = differentiateWithRespectToGradient(left, var_name);
      if (left_deriv)
        return multiply(right, left_deriv);
    }
    // If both have gradients, we'd need product rule - not handled yet
    return nullptr;
  }
  else if (expr->type() == NodeType::Gradient)
  {
    auto unary = std::static_pointer_cast<UnaryOpNode>(expr);
    auto operand = unary->operand();
    if (operand->type() == NodeType::Variable || operand->type() == NodeType::FieldVariable)
    {
      auto var_node = operand;
      if ((var_node->type() == NodeType::Variable && 
           std::static_pointer_cast<VariableNode>(var_node)->name() == var_name) ||
          (var_node->type() == NodeType::FieldVariable && 
           std::static_pointer_cast<FieldVariableNode>(var_node)->name() == var_name))
      {
        // ∂(grad(u))/∂(grad(u)) = I (identity tensor)
        return constant(1.0); // Simplified - should return identity tensor
      }
    }
  }
  else if (expr->type() == NodeType::Multiply)
  {
    auto binary = std::static_pointer_cast<BinaryOpNode>(expr);
    auto left = binary->left();
    auto right = binary->right();
    
    // Check if one side is grad(var) and the other is a constant/scalar
    NodePtr grad_part = nullptr;
    NodePtr coeff_part = nullptr;
    
    if (right->type() == NodeType::Gradient)
    {
      grad_part = right;
      coeff_part = left;
    }
    else if (left->type() == NodeType::Gradient)
    {
      grad_part = left;
      coeff_part = right;
    }
    
    if (grad_part)
    {
      auto unary = std::static_pointer_cast<UnaryOpNode>(grad_part);
      auto operand = unary->operand();
      if ((operand->type() == NodeType::Variable && 
           std::static_pointer_cast<VariableNode>(operand)->name() == var_name) ||
          (operand->type() == NodeType::FieldVariable && 
           std::static_pointer_cast<FieldVariableNode>(operand)->name() == var_name))
      {
        // ∂(κ * grad(u))/∂(grad(u)) = κ * I
        return coeff_part; // Returns the coefficient
      }
    }
  }
  
  // For more complex expressions, we would need a more sophisticated approach
  return nullptr;
}

template <bool is_ad>
bool
VariationalKernelBaseTempl<is_ad>::containsGradient(const NodePtr & expr, const std::string & var_name)
{
  if (!expr)
    return false;

  if (expr->type() == NodeType::Gradient)
  {
    auto unary = std::static_pointer_cast<UnaryOpNode>(expr);
    auto operand = unary->operand();
    if (operand->type() == NodeType::Variable || operand->type() == NodeType::FieldVariable)
    {
      if ((operand->type() == NodeType::Variable &&
           std::static_pointer_cast<VariableNode>(operand)->name() == var_name) ||
          (operand->type() == NodeType::FieldVariable &&
           std::static_pointer_cast<FieldVariableNode>(operand)->name() == var_name))
      {
        return true;
      }
    }
  }

  // Recursively check children
  for (const auto & child : expr->children())
  {
    if (containsGradient(child, var_name))
      return true;
  }

  return false;
}

// AD-compatible expression evaluation for strong forms
template <bool is_ad>
GenericReal<is_ad>
VariationalKernelBaseTempl<is_ad>::evaluateExpressionAD(const NodePtr & expr, unsigned int qp)
{
  if (!expr)
    return GenericReal<is_ad>(0.0);

  // Walk the AST and evaluate with AD types
  // Key insight: When is_ad=true, this->_u[qp], this->_grad_u[qp], _var.secondSln()[qp] are all ADReal
  // So operations automatically track derivatives!

  switch (expr->type())
  {
    case NodeType::Constant:
    {
      auto c = std::static_pointer_cast<ConstantNode>(expr);
      // ConstantNode::value() returns MooseValue, need to get scalar
      if (c->value().isScalar())
        return GenericReal<is_ad>(c->value().asScalar());  // Constants are Real (not differentiated)
      else
        this->mooseError("Non-scalar constant in AD evaluation");
    }

    case NodeType::Variable:
    case NodeType::FieldVariable:
    {
      // Field variable u at quadrature point
      std::string var_name;
      if (expr->type() == NodeType::Variable)
        var_name = std::static_pointer_cast<VariableNode>(expr)->name();
      else
        var_name = std::static_pointer_cast<FieldVariableNode>(expr)->name();

      if (var_name == _var.name())
        return this->_u[qp];  // ADReal when is_ad=true!
      else if (_coupled_values.count(var_name))
        return (*_coupled_values[var_name])[qp];  // Coupled variable
      else if (_parameters.count(var_name))
        return GenericReal<is_ad>(_parameters.at(var_name)); // parameter constant
      mooseError("Variable or parameter ", var_name, " not found");
    }

    case NodeType::Add:
    {
      auto binary = std::static_pointer_cast<BinaryOpNode>(expr);
      return evaluateExpressionAD(binary->left(), qp) + evaluateExpressionAD(binary->right(), qp);
    }

    case NodeType::Subtract:
    {
      auto binary = std::static_pointer_cast<BinaryOpNode>(expr);
      return evaluateExpressionAD(binary->left(), qp) - evaluateExpressionAD(binary->right(), qp);
    }

    case NodeType::Multiply:
    {
      auto binary = std::static_pointer_cast<BinaryOpNode>(expr);
      return evaluateExpressionAD(binary->left(), qp) * evaluateExpressionAD(binary->right(), qp);
    }

    case NodeType::Divide:
    {
      auto binary = std::static_pointer_cast<BinaryOpNode>(expr);
      return evaluateExpressionAD(binary->left(), qp) / evaluateExpressionAD(binary->right(), qp);
    }

    case NodeType::Power:
    {
      auto binary = std::static_pointer_cast<BinaryOpNode>(expr);
      auto base = evaluateExpressionAD(binary->left(), qp);
      auto exp = evaluateExpressionAD(binary->right(), qp);
      return std::pow(base, exp);
    }

    case NodeType::Negate:
    {
      auto unary = std::static_pointer_cast<UnaryOpNode>(expr);
      return -evaluateExpressionAD(unary->operand(), qp);
    }

    case NodeType::Laplacian:
    {
      // This is the KEY case: laplacian(u) evaluated from second derivatives
      auto unary = std::static_pointer_cast<UnaryOpNode>(expr);
      auto operand = unary->operand();

      std::string var_name;
      if (operand->type() == NodeType::Variable)
        var_name = std::static_pointer_cast<VariableNode>(operand)->name();
      else if (operand->type() == NodeType::FieldVariable)
        var_name = std::static_pointer_cast<FieldVariableNode>(operand)->name();
      else
        mooseError("Laplacian of non-variable not supported in AD evaluation");

      if (var_name == _var.name())
      {
        // laplacian(u) = trace(Hessian(u))
        // When is_ad=true, _var.secondSln()[qp] returns ADRankTwoTensor!
        // MOOSE AD automatically tracks ∂laplacian(u)/∂u_j
        const auto & second_u = _var.secondSln();
        return second_u[qp].tr();  // Trace gives laplacian - ADReal!
      }
      else if (_coupled_seconds.count(var_name))
      {
        return (*_coupled_seconds[var_name])[qp].tr();
      }
      mooseError("Variable ", var_name, " second derivatives not available for laplacian");
    }

    default:
      mooseError("Node type ", static_cast<int>(expr->type()),
                 " not yet implemented in evaluateExpressionAD");
  }

  return GenericReal<is_ad>(0.0);
}

// Explicit template instantiations
template class VariationalKernelBaseTempl<false>;
template class VariationalKernelBaseTempl<true>;

// Register both types
registerMooseObject("MooseApp", VariationalKernelBase);
registerMooseObject("MooseApp", ADVariationalKernelBase);

}
}
