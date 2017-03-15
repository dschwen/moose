/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PiecewiseFreeEnergy.h"
#include "libmesh/utility.h"

template <>
InputParameters
validParams<PiecewiseFreeEnergy>()
{
  InputParameters params = validParams<DerivativeFunctionMaterialBase>();
  params.addClassDescription("Material that implements a piecewise defined free energy with given "
                             "terminal concentrations and spinodal region");
  params.addRequiredCoupledVar("c", "Concentration variable");
  params.addParam<Real>("c0", 0.0, "Lower terminal concentration");
  params.addParam<Real>("c1", 1.0, "Upper terminal concentration");
  params.addParam<Real>(
      "spinodal_width", 0.9, "Width of the spinodal region in concentration space");
  params.addParam<Real>(
      "barrier_height", 0.5, "Height of the double well barrier");

  // third derivatives are not provided
  params.set<unsigned int>("derivative_order") = 2;
  return params;
}

PiecewiseFreeEnergy::PiecewiseFreeEnergy(const InputParameters & parameters)
  : DerivativeFunctionMaterialBase(parameters),
    _c(coupledValue("c")),
    _c0(getParam<Real>("c0")),
    _c1(getParam<Real>("c1")),
    _sw(getParam<Real>("spinodal_width")),
    _b(getParam<Real>("barrier_height"))
{
  // calculate coefficients
  _a = 4.0 * _b / (_c0 * _c0 - 2.0 * _c0 * _c1 + _c0 * _sw + _c1 * _c1 - _c1 * _sw);
  _d = -4.0 * _b / (_sw * (_c0 - _c1));
  _cb = (_c0 + _c1) / 2.0;

  // calculate boundaries of the spinodal
  _s0 = (_c0 + _c1 - _sw) / 2.0;
  _s1 = (_c0 + _c1 + _sw) / 2.0;
}

Real
PiecewiseFreeEnergy::computeF()
{
  const Real & c = _c[_qp];

  if (c < _s0)
    return _a * Utility::pow<2>(c - _c0);
  if (c > _s1)
    return _a * Utility::pow<2>(c - _c1);

  return _b - _d * Utility::pow<2>(c - _cb);
}

Real
PiecewiseFreeEnergy::computeDF(unsigned int /*j_var*/)
{
  const Real & c = _c[_qp];

  if (c < _s0)
    return _a * 2.0 * (c - _c0);
  if (c > _s1)
    return _a * 2.0 * (c - _c1);

  return -_d * 2.0 * (c - _cb);
}

Real
PiecewiseFreeEnergy::computeD2F(unsigned int /*j_var**/, unsigned int /*k_var*/)
{
  const Real & c = _c[_qp];

  if (c < _s0 || c > _s1)
    return _a * 2.0;

  return -_d * 2.0;
}

Real
PiecewiseFreeEnergy::computeD3F(unsigned int /*j_var*/, unsigned int /*k_var*/, unsigned int /*l_var*/)
{
  return 0.0;
}
