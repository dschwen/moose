//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiscreteNucleationData.h"

registerMooseObject("PhaseFieldApp", DiscreteNucleationData);

template <>
InputParameters
validParams<DiscreteNucleationData>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addClassDescription("Output diagnostic data on a DiscreteNucleationInserter");
  params.addRequiredParam<UserObjectName>("inserter", "DiscreteNucleationInserter user object");
  return params;
}

DiscreteNucleationData::DiscreteNucleationData(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _inserter(getUserObject<DiscreteNucleationInserter>("inserter")),
    _nucleus_list(_inserter.getNucleusList())
{
}

Real
DiscreteNucleationData::getValue()
{
  return _nucleus_list.size();
}
