#pragma once

#include "Kernel.h"

/**
 * Gray-Scott reaction term for variable u: -u*v^2 + F*(1-u)
 * This kernel must be manually coded with explicit Jacobian entries
 */
class GrayScottReactionU : public Kernel
{
public:
  static InputParameters validParams();
  GrayScottReactionU(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Coupled variable v
  const VariableValue & _v;

  /// Variable number for v (for off-diagonal Jacobian)
  unsigned int _v_var;

  /// Feed rate parameter
  const Real _F;
};
