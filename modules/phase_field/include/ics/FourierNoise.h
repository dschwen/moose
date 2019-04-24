#pragma once

#include "InitialCondition.h"

class FourierNoise;

template <>
InputParameters validParams<FourierNoise>();

/**
 * Generate noise using random fourier series coefficients
 */
class FourierNoise : public InitialCondition
{
public:
  FourierNoise(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  struct SeriesItem
  {
    RealVectorValue k;
    Real s;
    Real c;
  };

  const Real _dx;

  std::vector<SeriesItem> _series;
};
