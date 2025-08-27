//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWERDGReconstruction.h"

registerMooseObject("ShallowWaterApp", SWERDGReconstruction);

InputParameters
SWERDGReconstruction::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Face-extrapolated values for SWE variables [h,hu,hv] (stub: pass-through).");
  params.addRequiredCoupledVar("h", "Conserved variable: h");
  params.addRequiredCoupledVar("hu", "Conserved variable: h*u");
  params.addRequiredCoupledVar("hv", "Conserved variable: h*v");
  return params;
}

SWERDGReconstruction::SWERDGReconstruction(const InputParameters & parameters)
  : Material(parameters),
    _h(coupledValue("h")),
    _hu(coupledValue("hu")),
    _hv(coupledValue("hv")),
    _hf(declareProperty<Real>("h")),
    _huf(declareProperty<Real>("hu")),
    _hvf(declareProperty<Real>("hv"))
{
}

SWERDGReconstruction::~SWERDGReconstruction() {}

void
SWERDGReconstruction::computeQpProperties()
{
  // Stub: use cell-average values at face centers
  _hf[_qp] = _h[_qp];
  _huf[_qp] = _hu[_qp];
  _hvf[_qp] = _hv[_qp];
}

