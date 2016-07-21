/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef STABILIZEGRADIENT_H
#define STABILIZEGRADIENT_H

#include "Kernel.h"

class StabilizeGradient;

template<>
InputParameters validParams<StabilizeGradient>();

/**
 * Enforce the gradient of a variable to be zero along a given direction.
 */
class StabilizeGradient : public Kernel
{
public:
  StabilizeGradient(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  RealVectorValue _dir;

  const Real _epsilon;
};

#endif // STABILIZEGRADIENT_H
