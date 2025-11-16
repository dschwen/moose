#pragma once

#include "Kernel.h"

/**
 * Simple diffusion kernel with a coefficient: coef * ∇²u
 */
class CoefDiffusion : public Kernel
{
public:
  static InputParameters validParams();
  CoefDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Diffusion coefficient
  const Real _coef;
};
