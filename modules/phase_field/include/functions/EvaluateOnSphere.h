/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef EVALUATEONSPHERE_H
#define EVALUATEONSPHERE_H

#include "Function.h"

class EvaluateOnSphere;

template <>
InputParameters validParams<EvaluateOnSphere>();

/**
 *
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
  const bool _deg;
};

#endif // EVALUATEONSPHERE_H
