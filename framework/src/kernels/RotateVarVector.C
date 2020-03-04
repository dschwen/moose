#include "RotateVarVector.h"

registerMooseObject("MooseApp", RotateVarVector);

InputParameters
RotateVarVector::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Rotate a set of two variables as a vector");
  params.addCoupledVar("v", "The other vector component");
  params.addRequiredParam<Real>("factor", "component prefactor");
  return params;
}

RotateVarVector::RotateVarVector(const InputParameters & parameters)
  : Kernel(parameters),
    _u_old(valueOld()),
    _v(coupledValue("v")),
    _v_var(coupled("v")),
    _factor(getParam<Real>("factor"))
{
}

Real
RotateVarVector::computeQpResidual()
{
  return (_u_old[_qp] + _factor * _v[_qp] - _u[_qp]) * _test[_i][_qp];
}

Real
RotateVarVector::computeQpJacobian()
{
  return -_phi[_j][_qp] * _test[_i][_qp];
}

Real
RotateVarVector::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_var)
    return _factor * _phi[_j][_qp] * _test[_i][_qp];

  return 0.0;
}
