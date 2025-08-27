//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWEBathymetry.h"
#include "Function.h"

registerMooseObject("ShallowWaterApp", SWEBathymetry);

InputParameters
SWEBathymetry::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Provides bathymetry b as a material property.");
  params.addRequiredParam<FunctionName>("bed", "Bed elevation function b(x,y)");
  return params;
}

SWEBathymetry::SWEBathymetry(const InputParameters & parameters)
  : Material(parameters), _bed(getFunction("bed")), _b(declareProperty<Real>("b"))
{
}

SWEBathymetry::~SWEBathymetry() {}

void
SWEBathymetry::computeQpProperties()
{
  _b[_qp] = _bed.value(_t, _q_point[_qp]);
}
