#pragma once

#include "Function.h"
#include "FunctionInterface.h"

/**
 * Computes hydraulic diameter for a circular area from its area function
 */
class CircularAreaHydraulicDiameterFunction : public Function, public FunctionInterface
{
public:
  CircularAreaHydraulicDiameterFunction(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;
  virtual RealVectorValue gradient(Real t, const Point & p) const override;

protected:
  /// Area function
  const Function & _area_function;

public:
  static InputParameters validParams();
};
