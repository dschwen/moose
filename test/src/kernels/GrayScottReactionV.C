#include "GrayScottReactionV.h"

registerMooseObject("MooseTestApp", GrayScottReactionV);

InputParameters
GrayScottReactionV::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Gray-Scott reaction term for v: u*v^2 - (F+k)*v");
  params.addRequiredCoupledVar("u", "Coupled variable u");
  params.addRequiredParam<Real>("F", "Feed rate parameter");
  params.addRequiredParam<Real>("k", "Kill rate parameter");
  return params;
}

GrayScottReactionV::GrayScottReactionV(const InputParameters & parameters)
  : Kernel(parameters),
    _u_coupled(coupledValue("u")),
    _u_var(coupled("u")),
    _F(getParam<Real>("F")),
    _k(getParam<Real>("k"))
{
}

Real
GrayScottReactionV::computeQpResidual()
{
  // Residual for: u*v^2 - (F+k)*v
  Real u = _u_coupled[_qp];
  Real v = _u[_qp];  // Note: _u is the variable this kernel acts on (v)

  return (u * v * v - (_F + _k) * v) * _test[_i][_qp];
}

Real
GrayScottReactionV::computeQpJacobian()
{
  // Jacobian: ∂R/∂v = ∂(u*v^2 - (F+k)*v)/∂v
  //                  = 2*u*v - (F+k)
  Real u = _u_coupled[_qp];
  Real v = _u[_qp];

  return (2.0 * u * v - (_F + _k)) * _phi[_j][_qp] * _test[_i][_qp];
}

Real
GrayScottReactionV::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _u_var)
  {
    // Off-diagonal Jacobian: ∂R/∂u = ∂(u*v^2)/∂u = v^2
    Real v = _u[_qp];

    return (v * v) * _phi[_j][_qp] * _test[_i][_qp];
  }

  return 0.0;
}
