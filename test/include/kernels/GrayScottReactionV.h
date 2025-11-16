#pragma once

#include "Kernel.h"

/**
 * Gray-Scott reaction term for variable v: u*v^2 - (F+k)*v
 * This kernel must be manually coded with explicit Jacobian entries
 */
class GrayScottReactionV : public Kernel
{
public:
  static InputParameters validParams();
  GrayScottReactionV(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Coupled variable u
  const VariableValue & _u_coupled;

  /// Variable number for u (for off-diagonal Jacobian)
  unsigned int _u_var;

  /// Feed rate parameter
  const Real _F;

  /// Kill rate parameter
  const Real _k;
};
