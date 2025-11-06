#pragma once

#include "AuxKernel.h"

/**
 * Auxiliary kernel for computing gradient of a variable
 * Used for variable splitting in higher-order PDEs
 */
class GradientAuxKernel : public VectorAuxKernel
{
public:
  static InputParameters validParams();

  GradientAuxKernel(const InputParameters & parameters);

protected:
  virtual RealVectorValue computeValue() override;

private:
  /// The coupled variable whose gradient we're computing
  const VariableGradient & _grad_coupled;
  
  /// Component index for vector auxiliary variable
  const unsigned int _component;
};