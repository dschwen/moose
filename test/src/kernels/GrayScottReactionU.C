#include "GrayScottReactionU.h"

registerMooseObject("MooseTestApp", GrayScottReactionU);

InputParameters
GrayScottReactionU::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Gray-Scott reaction term for u: -u*v^2 + F*(1-u)");
  params.addRequiredCoupledVar("v", "Coupled variable v");
  params.addRequiredParam<Real>("F", "Feed rate parameter");
  return params;
}

GrayScottReactionU::GrayScottReactionU(const InputParameters & parameters)
  : Kernel(parameters),
    _v(coupledValue("v")),
    _v_var(coupled("v")),
    _F(getParam<Real>("F"))
{
}

Real
GrayScottReactionU::computeQpResidual()
{
  // Residual for: -u*v^2 + F*(1-u)
  Real u = _u[_qp];
  Real v = _v[_qp];

  return (-u * v * v + _F * (1.0 - u)) * _test[_i][_qp];
}

Real
GrayScottReactionU::computeQpJacobian()
{
  // Jacobian: ∂R/∂u = ∂(-u*v^2 + F*(1-u))/∂u
  //                  = -v^2 - F
  Real v = _v[_qp];

  return (-v * v - _F) * _phi[_j][_qp] * _test[_i][_qp];
}

Real
GrayScottReactionU::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_var)
  {
    // Off-diagonal Jacobian: ∂R/∂v = ∂(-u*v^2)/∂v = -2*u*v
    Real u = _u[_qp];
    Real v = _v[_qp];

    return (-2.0 * u * v) * _phi[_j][_qp] * _test[_i][_qp];
  }

  return 0.0;
}
