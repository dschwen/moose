//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

/**
 *
 */
class LagrangeInclinedNoDisplacement : public ADKernel
{
public:
  static InputParameters validParams();

  LagrangeInclinedNoDisplacement(const InputParameters & parameters);

protected:
  // update normals here
  void precalculateResidual() override;

  virtual ADReal computeQpResidual() override;

  /// normals
  const MooseArray<ADPoint> & _normals;

  /// displacement variables
  const std::vector<const ADVariableValue *> _disp;
};
