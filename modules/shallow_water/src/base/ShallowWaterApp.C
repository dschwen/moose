//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShallowWaterApp.h"
#include "ThermalHydraulicsApp.h"
#include "RdgApp.h"
#include "FluidPropertiesApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
ShallowWaterApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  params.set<bool>("automatic_automatic_scaling") = false;
  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;
  return params;
}

registerKnownLabel("ShallowWaterApp");

ShallowWaterApp::ShallowWaterApp(const InputParameters & parameters) : MooseApp(parameters)
{
  ShallowWaterApp::registerAll(_factory, _action_factory, _syntax);
}

ShallowWaterApp::~ShallowWaterApp() {}

void
ShallowWaterApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"ShallowWaterApp"});
  Registry::registerActionsTo(af, {"ShallowWaterApp"});

  ThermalHydraulicsApp::registerAll(f, af, s);
  FluidPropertiesApp::registerAll(f, af, s);
  RdgApp::registerAll(f, af, s);
}

void
ShallowWaterApp::registerApps()
{
  registerApp(ShallowWaterApp);
}

extern "C" void
ShallowWaterApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ShallowWaterApp::registerAll(f, af, s);
}
extern "C" void
ShallowWaterApp__registerApps()
{
  ShallowWaterApp::registerApps();
}
