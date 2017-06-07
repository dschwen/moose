//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MinMaxFunctionIC.h"

registerMooseObject("PhaseFieldApp", MinMaxFunctionIC);

template <>
InputParameters
validParams<MinMaxFunctionIC>()
{
  InputParameters params = validParams<FunctionIC>();
  params.addClassDescription(
      "Assign the maximum/minimum of either given coupled variable or a given function");
  params.addCoupledVar("v", "variable");
  MooseEnum minMaxEnum("min max", "max");
  params.addParam<MooseEnum>("value", minMaxEnum, "Which value to compute");
  return params;
}

MinMaxFunctionIC::MinMaxFunctionIC(const InputParameters & parameters)
  : FunctionIC(parameters), _v(coupledValue("v")), _max(getParam<MooseEnum>("value") == "max")
{
}

Real
MinMaxFunctionIC::value(const Point & p)
{
  return _max ? std::max(FunctionIC::value(p), _v[_qp]) : std::min(FunctionIC::value(p), _v[_qp]);
}
