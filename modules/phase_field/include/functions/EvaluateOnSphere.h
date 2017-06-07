//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef EVALUATEONSPHERE_H
#define EVALUATEONSPHERE_H

#include "Function.h"

class EvaluateOnSphere;

template <>
InputParameters validParams<EvaluateOnSphere>();

/**
 * Converts the passed in cartesian coordinates to spherical
 * coordinates and passes them through to another function.
 */
class EvaluateOnSphere : public Function
{
public:
  EvaluateOnSphere(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) override;

protected:
  FEProblemBase * _fe_problem;
  THREAD_ID _tid;

  Function & _function;
  const Real _radius;
  const Point _angle_scale;
  const bool _deg;
};

#endif // EVALUATEONSPHERE_H
