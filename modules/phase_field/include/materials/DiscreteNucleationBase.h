/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DISCRETENUCLEATIONBASE_H
#define DISCRETENUCLEATIONBASE_H

#include "DerivativeFunctionMaterialBase.h"

// Forward declaration
class DiscreteNucleationBase;
class DiscreteNucleationMap;

template<>
InputParameters validParams<DiscreteNucleationBase>();

/**
 * Free energy penalty contribution to force the nucleation of subresolution particles
 */
class DiscreteNucleationBase : public DerivativeFunctionMaterialBase
{
public:
  DiscreteNucleationBase(const InputParameters & params);

protected:
  /// Nucleation free energy prefactor
  const Real _penalty;

  /// Match concentration exactly or use the target as a minumum or maximum value
  const unsigned int _penalty_mode;

  /// UserObject providing a map of currently active nuclei
  const DiscreteNucleationMap & _map;
};

#endif //DISCRETENUCLEATIONBASE_H
