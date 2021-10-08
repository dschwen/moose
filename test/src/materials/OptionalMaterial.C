//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptionalMaterial.h"

registerMooseObject("MooseTestApp", OptionalMaterial);

InputParameters
OptionalMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("prop1", "Name of the first optional property");
  params.addRequiredParam<MaterialPropertyName>("prop2", "Name of the second optional property");
  params.addParam<MaterialPropertyName>("out", "optional_sum", "Name of the output property");
  return params;
}

OptionalMaterial::OptionalMaterial(const InputParameters & parameters)
  : Material(parameters),
    _mat_prop1(getOptionalMaterialPropertyByName<Real>(getParam<MaterialPropertyName>("prop1"))),
    _mat_prop2(getOptionalMaterialPropertyByName<Real>(getParam<MaterialPropertyName>("prop2"))),
    _out(declareProperty<Real>("out"))
{
}

void
OptionalMaterial::computeQpProperties()
{
  _out[_qp] = 0.0;
  if (_mat_prop1)
    _out[_qp] += (*_mat_prop1)[_qp];
  if (_mat_prop2)
    _out[_qp] += (*_mat_prop2)[_qp];
}
