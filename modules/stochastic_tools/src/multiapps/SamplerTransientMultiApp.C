/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// StochasticTools includes
#include "SamplerTransientMultiApp.h"
#include "Sampler.h"

template <>
InputParameters
validParams<SamplerTransientMultiApp>()
{
  InputParameters params = validParams<TransientMultiApp>();
  params += validParams<SamplerMultiAppInterface>();
  params.suppressParameter<std::vector<Point>>("positions");
  params.suppressParameter<bool>("output_in_position");
  params.suppressParameter<std::vector<FileName>>("positions_file");
  params.suppressParameter<Real>("move_time");
  params.suppressParameter<std::vector<Point>>("move_positions");
  params.suppressParameter<std::vector<unsigned int>>("move_apps");
  params.set<bool>("use_positions") = false;
  return params;
}

SamplerTransientMultiApp::SamplerTransientMultiApp(const InputParameters & parameters)
  : TransientMultiApp(parameters),
    SamplerInterface(this),
    SamplerMultiAppInterface(SamplerInterface::getSampler("sampler"))
{
  init(_sampler.getTotalNumberOfRows());
}
