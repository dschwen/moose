#include "LaplacianConstraintKernel.h"

registerMooseObject("MooseApp", LaplacianConstraintKernel);

InputParameters
LaplacianConstraintKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Enforces q = ∇²u (Laplacian constraint) for second-order variable splitting");
  params.addRequiredCoupledVar("u", "The variable whose Laplacian we're constraining to");
  return params;
}

LaplacianConstraintKernel::LaplacianConstraintKernel(const InputParameters & parameters)
  : Kernel(parameters),
    _grad_coupled(coupledGradient("u")),
    _coupled_var_num(coupled("u"))
{
}

Real
LaplacianConstraintKernel::computeQpResidual()
{
  // Weak form after integration by parts: (q * v + ∇u · ∇v, test) = 0
  // Residual: R = q * test + grad(u) · grad(test)
  return _u[_qp] * _test[_i][_qp] + _grad_coupled[_qp] * _grad_test[_i][_qp];
}

Real
LaplacianConstraintKernel::computeQpJacobian()
{
  // ∂R/∂q = test * φ
  return _phi[_j][_qp] * _test[_i][_qp];
}

Real
LaplacianConstraintKernel::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _coupled_var_num)
  {
    // ∂R/∂u = grad(φ) · grad(test)
    return _grad_phi[_j][_qp] * _grad_test[_i][_qp];
  }

  return 0.0;
}
