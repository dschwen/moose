/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "SamplerMultiAppInterface.h"

template <>
InputParameters
validParams<SamplerMultiAppInterface>()
{
  InputParameters params = emptyInputParameters();
  params.addClassDescription("Creates a sub-application for each row of each Sampler matrix.");
  params.addParam<SamplerName>("sampler", "The Sampler object to utilize for creating MultiApps.");
  return params;
}
