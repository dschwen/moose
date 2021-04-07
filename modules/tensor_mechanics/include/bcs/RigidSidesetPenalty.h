//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

class Function;
class RigidSideset;

/**
 * Apply a force in the direction of the rigidly transformed reference configuration.
 * This BC uses the average translation and rotation computed by the RigidSideset
 * UserObject to add forces that enforce the rigid body nature of a sideset.
 */
class RigidSidesetPenalty : public IntegratedBC
{
public:
  static InputParameters validParams();

  RigidSidesetPenalty(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const Function & _penalty;

  unsigned int _component;

  const RigidSideset & _rigid_sideset;
  const Point & _origin;
  const RealTensorValue & _rotation;
  const RealVectorValue & _shift;
};
