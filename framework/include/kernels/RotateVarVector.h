#pragma once

#include "Kernel.h"

/**
 *
 */
class RotateVarVector : public Kernel
{
public:
  static InputParameters validParams();

  RotateVarVector(const InputParameters & parameters);

protected:
  Real computeQpResidual() override;
  Real computeQpJacobian() override;
  Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const VariableValue & _u_old;
  const VariableValue & _v;
  unsigned int _v_var;
  const Real _factor;
};