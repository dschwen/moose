#include "FourierNoise.h"
#include "MooseRandom.h"

registerMooseObject("PhaseFieldApp", FourierNoise);

template <>
InputParameters
validParams<FourierNoise>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addClassDescription("");
  params.addRequiredParam<Real>("dx", "Element size");
  params.addParam<unsigned int>("length", 100, "Number of random fourier series terms");
  params.addParam<unsigned int>("seed", 12455, "Random number generator seed");
  return params;
}

FourierNoise::FourierNoise(const InputParameters & parameters)
  : InitialCondition(parameters),
    _dx(getParam<Real>("dx")),
    _series(getParam<unsigned int>("length"))
{
  MooseRandom::seed(getParam<unsigned int>("seed"));

  for (auto & f : _series)
  {
    Real r2;

    do
    {
      const Real x = MooseRandom::rand() - 0.5;
      const Real y = MooseRandom::rand() - 0.5;
      f.k = RealVectorValue(x, y, 0.0);
      r2 = f.k.norm_sq();
    } while (r2 > 0.25);

    f.k *= libMesh::pi / (2.0 * _dx);

    f.c = MooseRandom::randNormal(0, 1.0);
    f.s = MooseRandom::randNormal(0, 1.0);
  }
}

Real
FourierNoise::value(const Point & p)
{
  Real v = 0.0;
  for (const auto & f : _series)
    v += f.s * std::sin(p * f.k) + f.c * std::cos(p * f.k);
  return v;
}
