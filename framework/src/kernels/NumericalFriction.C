//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumericalFriction.h"
#include "libmesh/utility.h"

registerMooseObject("MooseApp", NumericalFriction);

InputParameters
NumericalFriction::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Kernel that adds a penalty based numerical friction.");
  params.addParam<Real>("penalty", 1e-9, "Penalty factor to scale the variable increment.");
  return params;
}

NumericalFriction::NumericalFriction(const InputParameters & parameters)
  : Kernel(parameters), _u_old(_var.slnOld()), _penalty(getParam<Real>("penalty"))
{
}

Real
NumericalFriction::computeQpResidual()
{
  return _penalty * Utility::pow<2>(_u[_qp] - _u_old[_qp]) * _test[_i][_qp];
}

Real
NumericalFriction::computeQpJacobian()
{
  return _penalty * 2.0 * (_u[_qp] - _u_old[_qp]) * _phi[_j][_qp] * _test[_i][_qp];
}
