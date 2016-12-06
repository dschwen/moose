/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DISCRETENUCLEATIONPARSED_H
#define DISCRETENUCLEATIONPARSED_H

#include "DiscreteNucleationBase.h"

// Forward declaration
class DiscreteNucleationParsed;

template<>
InputParameters validParams<DiscreteNucleationParsed>();

/**
 * Free energy penalty contribution to force the nucleation of subresolution particles
 */
class DiscreteNucleationParsed : public DiscreteNucleationBase
{
public:
  DiscreteNucleationParsed(const InputParameters & params);

  virtual void computeProperties();

protected:
  /// Target material property value
  const Real _F_value;
};

#endif //DISCRETENUCLEATIONPARSED_H
