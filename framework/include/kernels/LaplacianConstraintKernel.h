#pragma once

#include "Kernel.h"

/**
 * Laplacian constraint kernel for second-order variable splitting
 * Enforces q = ∇²u (Laplacian of u)
 *
 * Weak form after integration by parts:
 * ∫ (q * v + ∇u · ∇v) dx = 0
 *
 * This is used for splitting 4th-order PDEs into systems of 2nd-order equations
 */
class LaplacianConstraintKernel : public Kernel
{
public:
  static InputParameters validParams();

  LaplacianConstraintKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

private:
  /// The gradient of the coupled variable
  const VariableGradient & _grad_coupled;

  /// Variable number of the coupled variable
  unsigned int _coupled_var_num;
};
