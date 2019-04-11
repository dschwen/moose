//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADParsedMaterialBase.h"

registerADMooseObject("PhaseFieldTestApp", ADParsedMaterialBase);

defineADValidParams(
    ADParsedMaterialBase,
    ADMaterial,
    params.addClassDescription("Base class for parsed materials using ExprTk.");
    params.addRequiredParam<std::string>("function", "Function expression using ExprTk syntax");
    params.addParam<MaterialPropertyName>("f_name", "F", "function property name");
    params.addRequiredCoupledVar("args", "Coupled variable parameter variables");
    params.addParam<std::vector<std::string>>(
        "variable_names",
        "Names under which the coupled variables appear in the function expression. If omitted "
        "they appear under their original MOOSE variable names."););

template <ComputeStage compute_stage>
ADParsedMaterialBase<compute_stage>::ADParsedMaterialBase(const InputParameters & parameters)
  : ADMaterial<compute_stage>(parameters),
    _function(adGetParam<std::string>("function")),
    _args(coupledComponents("args")),
    _var_names(adGetParam<std::vector<std::string>>("variable_names")),
    _f_name(adGetParam<MaterialPropertyName>("f_name")),
    _prop_F(adDeclareADProperty<Real>(_f_name))
{
  // error check
  if (!_var_names.empty() && _var_names.size() != _args.size())
    paramError("variable_names",
               "Either specify as many names here as 'args' or none at all (to default to the "
               "original args names)");

  // create material properties, couple variables, add buffer to expression
  for (std::size_t i = 0; i < _args.size(); ++i)
    _args[i] = &adCoupledValue("op", i);
}

adBaseClass(ADParsedMaterialBase);
