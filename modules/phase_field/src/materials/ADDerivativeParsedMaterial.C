//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDerivativeParsedMaterial.h"

registerADMooseObject("PhaseFieldTestApp", ADDerivativeParsedMaterial);

defineADValidParams(
    ADDerivativeParsedMaterial,
    ADParsedMaterialBase,
    params.addClassDescription("Material that provides values from a parsed function and its first "
                               "derivatives, and derivatives w.r.t. all DOFs for them."););

template <ComputeStage compute_stage>
ADDerivativeParsedMaterial<compute_stage>::ADDerivativeParsedMaterial(
    const InputParameters & parameters)
  : ADParsedMaterialBase<compute_stage>(parameters),
    _args_buffer(_args.size()),
    _prop_dFdargs(_args.size()),
    _exprtk()
{
  // create material properties, couple variables, add buffer to expression
  for (std::size_t i = 0; i < _args.size(); ++i)
  {
    auto iname = this->getVar("args", i)->name();
    _prop_dFdargs[i] = &adDeclareADProperty<Real>(derivativePropertyNameFirst(_f_name, iname));
    _exprtk.addVariable(_var_names.empty() ? iname : _var_names[i], _args_buffer[i]);
  }
}

template <ComputeStage compute_stage>
void
ADDerivativeParsedMaterial<compute_stage>::computeQpProperties()
{
  // copy over variables
  for (std::size_t i = 0; i < _args.size(); ++i)
    _args_buffer[i] = (*_args[i])[_qp];

  // evaluate function
  _prop_F[_qp] = _exprtk.value();

  // copy back derivatives
  for (std::size_t i = 0; i < _args.size(); ++i)
    (*_prop_dFdargs[i])[_qp] = 0.0;
}
