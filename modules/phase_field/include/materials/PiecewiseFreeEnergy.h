/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PIECEWISEFREEENERGY_H
#define PIECEWISEFREEENERGY_H

#include "DerivativeFunctionMaterialBase.h"

//Forward Declarations
class PiecewiseFreeEnergy;

template<>
InputParameters validParams<PiecewiseFreeEnergy>();

/**
 * Material class that creates a piecewise defined double well free energy
 * with specified terminal concentrations \f$ c_0, c_1 \f$, a spinodal region
 * of width \f$ s_w \f$, and a barrier height of \f$ b \f$
 */
class PiecewiseFreeEnergy : public DerivativeFunctionMaterialBase
{
public:
  PiecewiseFreeEnergy(const InputParameters & parameters);

protected:
  virtual Real computeF();
  virtual Real computeDF(unsigned int j_var);
  virtual Real computeD2F(unsigned int j_var, unsigned int k_var);
  virtual Real computeD3F(unsigned int j_var, unsigned int k_var, unsigned int l_var);

private:
  /// Coupled variable value for the concentration \f$ c \f$.
  const VariableValue & _c;

  /// Terminal concentrations
  const Real _c0;
  const Real _c1;
  ///@}

  /// Spinodal width
  const Real _sw;

  /// Barrier height
  const Real _b;

  ///@{ coefficients
  Real _a;
  Real _d;
  Real _cb;
  ///@}

  ///@{ boundaries of the spinodal
  Real _s0;
  Real _s1;
  ///@}
};

#endif //PIECEWISEFREEENERGY_H
