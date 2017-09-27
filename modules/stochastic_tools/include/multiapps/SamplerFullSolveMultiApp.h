/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef SAMPLERFULLSOLVEMULTIAPP_H
#define SAMPLERFULLSOLVEMULTIAPP_H

// MOOSE includes
#include "FullSolveMultiApp.h"
#include "SamplerMultiAppInterface.h"
#include "SamplerInterface.h"

class SamplerFullSolveMultiApp;

template <>
InputParameters validParams<SamplerFullSolveMultiApp>();

class SamplerFullSolveMultiApp : public FullSolveMultiApp,
                                 public SamplerInterface,
                                 public SamplerMultiAppInterface
{
public:
  SamplerFullSolveMultiApp(const InputParameters & parameters);
};

#endif // SAMPLERFULLSOLVEMULTIAPP_H
