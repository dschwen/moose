/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef COEFLAPLACIANSPLIT_H
#define COEFLAPLACIANSPLIT_H

#include "LaplacianSplit.h"

// Forward Declarations
class CoefLaplacianSplit;

template <>
InputParameters validParams<CoefLaplacianSplit>();

/**
 * Split with a variable that holds the Laplacian of the phase field
 * multiplied by a coefficient.
 */
class CoefLaplacianSplit : public LaplacianSplit
{
public:
  CoefLaplacianSplit(const InputParameters & parameters);

protected:
  virtual RealGradient precomputeQpResidual() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Laplacian pre-factor
  const MaterialProperty<Real> & _coef;
};

#endif // COEFLAPLACIANSPLIT_H
