/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ACINTERFACESTRESS_H
#define ACINTERFACESTRESS_H

#include "Kernel.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

class ACInterfaceStress;

template <>
InputParameters validParams<ACInterfaceStress>();

/**
 * Compute the Allen-Cahn interface term with the weak form residual
 * \f$ \left( \kappa_i \nabla\eta_i, \nabla (L_i \psi) \right) \f$
 */
class ACInterfaceStress : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  ACInterfaceStress(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Mobility
  const MaterialProperty<Real> & _L;

  /// d sigma/d(grad eta), derivative of interface stress tensor with order parameter gradient
  RankThreeTensor _dS;
};

#endif // ACINTERFACESTRESS_H
