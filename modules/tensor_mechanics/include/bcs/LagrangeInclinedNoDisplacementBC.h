//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

/**
 *
 */
class LagrangeInclinedNoDisplacementBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  LagrangeInclinedNoDisplacementBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  unsigned int _component;

  // const std::vector<const ADVariableValue *> _disp;
  const ADVariableValue & _lambda;
};
