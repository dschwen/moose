//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GradSquare.h"
#include "MooseVariable.h"

registerMooseObject("PhaseFieldApp", GradSquare);

template <>
InputParameters
validParams<GradSquare>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Term representing the square of a (coupled) variable gradient");
  params.addCoupledVar("v",
                       "Set this to make v a coupled variable, otherwise it will use the "
                       "kernel's nonlinear variable for v");
  return params;
}

GradSquare::GradSquare(const InputParameters & parameters)
  : Kernel(parameters),
    _is_coupled(isCoupled("v")),
    _grad_v(_is_coupled ? coupledGradient("v") : _grad_u),
    _v_var(_is_coupled ? coupled("v") : _var.number())
{
}

Real
GradSquare::computeQpResidual()
{
  return 0.5 * _test[_i][_qp] * _grad_v[_qp] * _grad_v[_qp];
}

Real
GradSquare::computeQpJacobian()
{
  if (_is_coupled)
    return 0.0;

  return _test[_i][_qp] * _grad_phi[_j][_qp] * _grad_v[_qp];
}

Real
GradSquare::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_var && _is_coupled)
    return _test[_i][_qp] * _grad_phi[_j][_qp] * _grad_v[_qp];

  return 0.0;
}
