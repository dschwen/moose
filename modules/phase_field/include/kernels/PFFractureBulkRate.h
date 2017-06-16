/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PFFRACTUREBULKRATE_H
#define PFFRACTUREBULKRATE_H

#include "PFFractureBulkRateBase.h"

// Forward Declarations
class PFFractureBulkRate;

template <>
InputParameters validParams<PFFractureBulkRate>();

/**
 * Phase field based fracture model
 * This kernel computes the residual and Jacobian for bulk free energy contribution to c
 * Refer to Formulation: Miehe et. al., Int. J. Num. Methods Engg., 2010, 83. 1273-1311 Equation 63
 */
class PFFractureBulkRate : public PFFractureBulkRateBase
{
public:
  PFFractureBulkRate(const InputParameters & parameters);

protected:
  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// second derivative of the kernel variable
  const VariableSecond & _u_second;
};

#endif // PFFRACTUREBULKRATE_H
