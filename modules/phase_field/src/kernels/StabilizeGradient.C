/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "StabilizeGradient.h"

template<>
InputParameters validParams<StabilizeGradient>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Enforce the gradient of a variable to be zero along a given direction ");
  params.addRequiredParam<RealVectorValue>("direction", "Direction to suppress gradient of the variable in");
  params.addParam<Real>("epsilon", 1e-9, "Jacobian fill term magnitude");
  return params;
}

StabilizeGradient::StabilizeGradient(const InputParameters & parameters) :
    Kernel(parameters),
    _dir(getParam<RealVectorValue>("direction")),
    _epsilon(getParam<Real>("epsilon"))
{
}

Real
StabilizeGradient::computeQpResidual()
{
  return   (_grad_u[_qp] * _dir) * _test[_i][_qp]
         + _epsilon * (_grad_u[_qp] * _grad_test[_i][_qp]);
}

Real
StabilizeGradient::computeQpJacobian()
{
  return   (_grad_phi[_j][_qp] * _dir) * _test[_i][_qp]
         + _epsilon * (_grad_phi[_j][_qp] * _grad_test[_i][_qp]);
}
