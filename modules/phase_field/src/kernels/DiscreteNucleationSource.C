//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiscreteNucleationSource.h"
#include "DiscreteNucleationMap.h"

registerMooseObject("PhaseFieldApp", DiscreteNucleationSource);

template <>
InputParameters
validParams<DiscreteNucleationSource>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription(
      "Source term for the nucleating grains or phases in non-conserved order parameter fields");
  params.addRequiredParam<UserObjectName>("map", "DiscreteNucleationMap user object");
  return params;
}

DiscreteNucleationSource::DiscreteNucleationSource(const InputParameters & params)
  : Kernel(params), _map(getUserObject<DiscreteNucleationMap>("map"))
{
}

void
DiscreteNucleationSource::precalculateResidual()
{
  // check if a nucleation event list is available for the current element
  _nucleus = &_map.nuclei(_current_elem);
}

Real
DiscreteNucleationSource::computeQpResidual()
{
  return (_u[_qp] - (*_nucleus)[_qp]) * _test[_i][_qp];
}

Real
DiscreteNucleationSource::computeQpJacobian()
{
  return _phi[_j][_qp] * _test[_i][_qp];
}
