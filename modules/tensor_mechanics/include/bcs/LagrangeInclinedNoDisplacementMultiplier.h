#pragma once

#include "ADIntegratedBC.h"

/**
 *
 */
class LagrangeInclinedNoDisplacementMultiplier : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  LagrangeInclinedNoDisplacementMultiplier(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  const std::vector<const ADVariableValue *> _disp;
};
