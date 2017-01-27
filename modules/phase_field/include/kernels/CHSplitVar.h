/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CHSPLITVAR_H
#define CHSPLITVAR_H

#include "KernelGrad.h"

//Forward Declarations
class CHSplitVar;

template<>
InputParameters validParams<CHSplitVar>();

/**
 *
 */
class CHSplitVar : public KernelGrad
{
public:
  CHSplitVar(const InputParameters & parameters);

protected:
  virtual RealGradient precomputeQpResidual() override;
  virtual RealGradient precomputeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

private:
  Real _var_c;
  const VariableGradient & _grad_c;
};

#endif //CHSPLITVAR_H
