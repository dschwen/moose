//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LagrangeInclinedNoDisplacementBC.h"

registerMooseObject("TensorMechanicsApp", LagrangeInclinedNoDisplacementBC);

InputParameters
LagrangeInclinedNoDisplacementBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addCoupledVar("lambda", "Lagrange multiplier variable");
  params.addRequiredParam<unsigned int>("component", ".");
  return params;
}

LagrangeInclinedNoDisplacementBC::LagrangeInclinedNoDisplacementBC(
    const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _component(getParam<unsigned int>("component")),
    _lambda(adCoupledValue("lambda"))
{
}

ADReal
LagrangeInclinedNoDisplacementBC::computeQpResidual()
{
  return _lambda[_qp] * _normals[_qp](_component) * _test[_i][_qp];
}
