//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DisplacedBodyForce.h"
#include "Function.h"

registerMooseObject("MooseApp", DisplacedBodyForce);

template <>
InputParameters
validParams<DisplacedBodyForce>()
{
  InputParameters params = validParams<ALEKernel>();
  params.addClassDescription(
      "BodyForce kernel with off diagonal Jacobians w.r.t. to displacements.");
  params.addParam<Real>("value", 1.0, "Coefficient to multiply by the body force term");
  params.addParam<FunctionName>("function", "1", "A function that describes the body force");
  params.addParam<PostprocessorName>(
      "postprocessor", 1, "A postprocessor whose value is multiplied by the body force");
  params.declareControllable("value");
  return params;
}

DisplacedBodyForce::DisplacedBodyForce(const InputParameters & parameters)
  : ALEKernel(parameters),
    _scale(getParam<Real>("value")),
    _function(getFunction("function")),
    _postprocessor(getPostprocessorValue("postprocessor"))
{
  // if (!getParam<bool>("use_displaced_mesh"))
  //   paramError("use_displaced_mesh", "This kernel is only valid on the displaced mesh");
}

Real
DisplacedBodyForce::computeQpResidual()
{
  Real factor = _scale * _postprocessor * _function.value(_t, _q_point[_qp]);
  return _test[_i][_qp] * -factor;
}
