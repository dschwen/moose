/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef IDEALGASFREEENERGY_H
#define IDEALGASFREEENERGY_H

#include "DerivativeParsedMaterialHelper.h"
#include "ExpressionBuilder.h"

// Forward Declarations
class IdealGasFreeEnergy;

template <>
InputParameters validParams<IdealGasFreeEnergy>();

/**
 * Material class that provides the free energy of an ideal gas with the expression builder
 * and uses automatic differentiation to get the derivatives.
 */
class IdealGasFreeEnergy : public DerivativeParsedMaterialHelper, public ExpressionBuilder
{
public:
  IdealGasFreeEnergy(const InputParameters & parameters);

protected:
  /// Coupled variable value for the Temperature
  EBTerm _T;

  /// Coupled variable value for the concentration \f$ c \f$.
  EBTerm _c;

  ///@{ physical constants
  const Real _hbar;
  const Real _kB;
  ///@}
};

#endif // IDEALGASFREEENERGY_H
