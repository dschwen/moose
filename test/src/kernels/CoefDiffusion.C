#include "CoefDiffusion.h"

registerMooseObject("MooseTestApp", CoefDiffusion);

InputParameters
CoefDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Diffusion with coefficient: coef * ∇²u");
  params.addRequiredParam<Real>("coef", "Diffusion coefficient");
  return params;
}

CoefDiffusion::CoefDiffusion(const InputParameters & parameters)
  : Kernel(parameters),
    _coef(getParam<Real>("coef"))
{
}

Real
CoefDiffusion::computeQpResidual()
{
  // Weak form of coef * ∇²u: coef * ∇u · ∇ψ
  return _coef * _grad_u[_qp] * _grad_test[_i][_qp];
}

Real
CoefDiffusion::computeQpJacobian()
{
  // Jacobian: coef * ∇φ_j · ∇ψ_i
  return _coef * _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}
