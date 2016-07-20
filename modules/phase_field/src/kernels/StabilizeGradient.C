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
  params.addClassDescription("Enforce the gradient of a variable to be zero along a given direction");
  params.addRequiredParam<RealVectorValue>("direction", "Direction to suppress gradient of the variable in");
  return params;
}

StabilizeGradient::StabilizeGradient(const InputParameters & parameters) :
    Kernel(parameters),
    _dir(getParam<RealVectorValue>("direction"))
{
}

Real
StabilizeGradient::computeQpResidual()
{
  return std::pow(_grad_u[_qp] * _dir, 2.0) * _test[_i][_qp];
}

Real
StabilizeGradient::computeQpJacobian()
{
  return 2.0 * (_grad_u[_qp] * _dir) * (_grad_phi[_j][_qp] * _dir) * _test[_i][_qp];
}
