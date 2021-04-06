//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideUserObject.h"

/**
 * Compute the average translation and rotation of a sideset.
 */
class RigidSideset : public SideUserObject
{
public:
  static InputParameters validParams();

  RigidSideset(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  virtual void threadJoin(const UserObject & y) override;

  const Point & getOrigin() const { return _origin; }
  const RealTensorValue & getRotation() const { return _rotation; }
  const RealVectorValue & getShift() const { return _shift; }

protected:
  /// qp data
  struct QPData
  {
    QPData(Point q_point, RealVectorValue delta, Real weight)
      : _q_point(q_point), _delta(delta), _weight(weight)
    {
    }
    Point _q_point;
    RealVectorValue _delta;
    Real _weight;
  };

  /// list of all quadrature points and their coordinates with weights
  std::vector<QPData> _qpdata;

  /// origin point to determine the angle w.r.t.
  const Point _origin;

  /// displacement variables
  std::vector<const VariableValue *> _disp;

  /// averaging translation
  RealVectorValue _shift;
  Real _weight;

  /// rotation matrix
  RealTensorValue _rotation;
};
