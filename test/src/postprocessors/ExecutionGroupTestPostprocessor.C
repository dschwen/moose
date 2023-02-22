//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExecutionGroupTestPostprocessor.h"

registerMooseObject("MooseTestApp", ExecutionGroupTestPostprocessor);

InputParameters
ExecutionGroupTestPostprocessor::validParams()
{
  InputParameters params = ElementIntegralVariablePostprocessor::validParams();
  params.addClassDescription(
      "Chainable element postprocessor for checking execution order grouping");
  params.addParam<PostprocessorName>("chained_postprocessor", "Postprocessor to add data from");
  return params;
}

ExecutionGroupTestPostprocessor::ExecutionGroupTestPostprocessor(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters),
    _chained_object(isParamValid("chained_postprocessor")
                        ? &getUserObject<ExecutionGroupTestPostprocessor>("chained_postprocessor")
                        : nullptr)
{
}

Real
ExecutionGroupTestPostprocessor::computeQpIntegral()
{
  return ElementIntegralVariablePostprocessor::computeQpIntegral() +
         (_chained_object ? _chained_object->getData() : 0.0);
}

void
ExecutionGroupTestPostprocessor::finalize()
{
  ElementIntegralVariablePostprocessor::finalize();
  _final_integral = _integral_value;
}
