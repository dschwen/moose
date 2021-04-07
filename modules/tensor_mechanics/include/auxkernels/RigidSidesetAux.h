//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class RigidSideset;

/**
 * Compute the distance of a sideset to the rigidly transformed reference configuration.
 */
class RigidSidesetAux : public AuxKernel
{
public:
  static InputParameters validParams();

  RigidSidesetAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// displacement variables
  std::vector<const VariableValue *> _disp;

  const RigidSideset & _rigid_sideset;
  const Point & _origin;
  const RealTensorValue & _rotation;
  const RealVectorValue & _shift;
};
