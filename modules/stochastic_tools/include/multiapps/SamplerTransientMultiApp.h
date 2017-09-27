/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef SAMPLERTRANSIENTMULTIAPP_H
#define SAMPLERTRANSIENTMULTIAPP_H

// MOOSE includes
#include "TransientMultiApp.h"
#include "SamplerMultiAppInterface.h"
#include "SamplerInterface.h"

class SamplerTransientMultiApp;

template <>
InputParameters validParams<SamplerTransientMultiApp>();

class SamplerTransientMultiApp : public TransientMultiApp,
                                 public SamplerInterface,
                                 public SamplerMultiAppInterface
{
public:
  SamplerTransientMultiApp(const InputParameters & parameters);
};

#endif // SAMPLERTRANSIENTMULTIAPP_H
