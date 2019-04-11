//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADParsedMaterial.h"

registerADMooseObject("PhaseFieldTestApp", ADParsedMaterial);

defineADValidParams(
    ADParsedMaterial,
    ADParsedMaterialBase,
    params.addClassDescription("Material that provides values from a parsed function and its "
                               "derivatives with respect to all DOFs."););

template <ComputeStage compute_stage>
ADParsedMaterial<compute_stage>::ADParsedMaterial(const InputParameters & parameters)
  : ADParsedMaterialBase<compute_stage>(parameters), _args_buffer(_args.size()), _exprtk()
{
  // create material properties, couple variables, add buffer to expression
  for (std::size_t i = 0; i < _args.size(); ++i)
  {
    auto iname = this->getVar("args", i)->name();
    _exprtk.addVariable(_var_names.empty() ? iname : _var_names[i], _args_buffer[i]);
  }
}

template <ComputeStage compute_stage>
void
ADParsedMaterial<compute_stage>::computeQpProperties()
{
  // copy over variables
  for (std::size_t i = 0; i < _args.size(); ++i)
    _args_buffer[i] = (*_args[i])[_qp];

  // evaluate function
  _prop_F[_qp] = _exprtk.value();
}
