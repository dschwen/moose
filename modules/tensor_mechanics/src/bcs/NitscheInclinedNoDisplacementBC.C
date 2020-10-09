//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NitscheInclinedNoDisplacementBC.h"

registerMooseObject("TensorMechanicsApp", NitscheInclinedNoDisplacementBC);

InputParameters
NitscheInclinedNoDisplacementBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<unsigned int>(
      "component", "An integer corresponding to the direction (0 for x, 1 for y, 2 for z)");
  params.addParam<MaterialPropertyName>("stress", "Name of the stress tensor property");
  return params;
}

NitscheInclinedNoDisplacementBC::NitscheInclinedNoDisplacementBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _component(getParam<unsigned int>("component")),
    _stress(getMaterialProperty<RankTwoTensor>("stress"))
{
}

Real
NitscheInclinedNoDisplacementBC::computeQpResidual()
{
  auto pressure = _normals[_qp] * _stress[_qp] * _normals[_qp];
  return -pressure * _normals[_qp](_component) * _test[_i][_qp];
}
