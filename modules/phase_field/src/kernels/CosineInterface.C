#include "CosineInterface.h"

template<>
InputParameters validParams<CosineInterface>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

CosineInterface::CosineInterface(const InputParameters & parameters) :
    Kernel(parameters),
    _a(1.0)
{
}

Real
CosineInterface::computeQpResidual()
{
  Real target = 0.0;
  if (_u[_qp] < 1.0 && _u[_qp] > 0)
    target = -_a / 2.0 * std::sin(std::acos(2.0 * _u[_qp] - 1.0));

  return (_grad_u[_qp].norm() - target) * _test[_i][_qp];
}

Real
CosineInterface::computeQpJacobian()
{
  Real dTarget = 0.0;
  // if (_u[_qp] < 1.0 && _u[_qp] > 0)
  //   dTtarget = -_a / 2.0 * std::sin(std::acos(2.0 * _u[_qp] - 1.0));

  return (2.0 * _grad_phi[_j][_qp] * _grad_u[_qp] - dTarget) * _test[_i][_qp];
}
