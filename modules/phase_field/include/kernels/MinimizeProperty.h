/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MINIMIZEPROPERTY_H
#define MINIMIZEPROPERTY_H

#include "Kernel.h"
#include "DerivativeMaterialInterface.h"
#include "JvarMapInterface.h"

class MinimizeProperty;

template<>
InputParameters validParams<MinimizeProperty>();

/**
 * Solves for a variable to minimize a given material property. Can be used for
 * optimization problems with DerivativeParsedMaterial objects.
 */
class MinimizeProperty : public DerivativeMaterialInterface<JvarMapInterface<Kernel>>
{
public:
  MinimizeProperty(const InputParameters & parameters);

  virtual void initialSetup();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Derivative of the property to be minimized w.r.t. the kernel variable
  const MaterialProperty<Real> & _dPdu;

  /// Second derivative of the property to be minimized w.r.t. the kernel variable
  const MaterialProperty<Real> & _d2Pdu2;

  /// Derivative of the property to be minimized w.r.t. the kernel variable and the additional arguments
  std::vector<const MaterialProperty<Real> *> _d2Pduarg;
};

#endif // MINIMIZEPROPERTY_H
