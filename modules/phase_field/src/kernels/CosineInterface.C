#include "CosineInterface.h"

template<>
InputParameters validParams<CosineInterface>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<RealVectorValue>("direction", "Direction perpendicular to the the enforced interface profile");
  return params;
}

CosineInterface::CosineInterface(const InputParameters & parameters) :
    Kernel(parameters),
    _direction(getParam<RealVectorValue>("direction")),
    _a(1.0)
{
}

Real
CosineInterface::computeQpResidual()
{
  Real target = 0.0;
  if (_u[_qp] > 0 && _u[_qp] < 1.0)
    target = -_a / 2.0 * std::sin(std::acos(2.0 * _u[_qp] - 1.0));

  // return (_grad_u[_qp].norm() - target) * _test[_i][_qp];
  return (_grad_u[_qp] * _direction - target) * _test[_i][_qp];
}

Real
CosineInterface::computeQpJacobian()
{
  Real dTarget = 0.0;
  if (_u[_qp] > 0 && _u[_qp] < 1.0)
    dTarget = _a * (2.0 * _u[_qp] - 1.0) * _phi[_j][_qp] / std::sqrt(-std::pow(2.0 * _u[_qp] - 1.0, 2.0) + 1.0);

  return (_grad_phi[_j][_qp] * _direction - dTarget) * _test[_i][_qp];
}
