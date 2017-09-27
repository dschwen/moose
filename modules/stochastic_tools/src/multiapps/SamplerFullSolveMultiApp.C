/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// StochasticTools includes
#include "SamplerFullSolveMultiApp.h"
#include "Sampler.h"

template <>
InputParameters
validParams<SamplerFullSolveMultiApp>()
{
  InputParameters params = validParams<FullSolveMultiApp>();
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

SamplerFullSolveMultiApp::SamplerFullSolveMultiApp(const InputParameters & parameters)
  : FullSolveMultiApp(parameters),
    SamplerInterface(this),
    SamplerMultiAppInterface(SamplerInterface::getSampler("sampler"))
{
  init(_sampler.getTotalNumberOfRows());
}
