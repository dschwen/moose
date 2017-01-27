/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PFCRFFEnergyDensity.h"
#include "libmesh/utility.h"

template<>
InputParameters validParams<PFCRFFEnergyDensity>()
{
   InputParameters params = validParams<AuxKernel>();
   params.addRequiredCoupledVar( "v", "Array of coupled variables" );
   params.addParam<std::vector<Real>>( "coeffs", "Modified Coefficients in Taylor Series Expansion (c a b)");
   MooseEnum log_options("tolerance cancelation expansion nothing");
   params.addRequiredParam<MooseEnum>("log_approach", log_options, "Which approach will be used to handle the natural log");
   params.addParam<Real>("tol", 1.0e-9, "Tolerance used when the tolerance approach is chosen");
   return params;
}

PFCRFFEnergyDensity::PFCRFFEnergyDensity(const InputParameters & parameters) :
    AuxKernel(parameters),
    _order(coupledComponents("v")),
    _coeffs(getParam<std::vector<Real>>("coeffs")),
    _log_approach(getParam<MooseEnum>("log_approach")),
    _tol(getParam<Real>("tol"))
{
  _vals.resize(_order);
  for (unsigned int i = 0; i < _order; ++i)
    _vals[i] = &coupledValue("v", i);
}

Real
PFCRFFEnergyDensity::computeValue()
{
  Real val = 0.0;
  switch (_log_approach)
  {
    case 0: // approach using tolerence
      if (1.0 + (*_vals[0])[_qp] < _tol)
        val += ((1.0 + _tol) * std::log(1 + _tol)) - _tol;
      else
        val += ((1.0 + (*_vals[0])[_qp]) * std::log(1 + (*_vals[0])[_qp])) - (*_vals[0])[_qp];
      break;

    case 1: // approach using cancellation
      val += ((1.0 + (*_vals[0])[_qp]) * std::log(1.0 + (*_vals[0])[_qp])) - (*_vals[0])[_qp];
      break;

    case 2: // approach using Taylor Series Expansion
      Real p = 2.0;
      for (auto coeff : _coeffs)
      {
        val += coeff * (std::pow(-1.0, p) / (p * (p - 1.0))) * std::pow((*_vals[0])[_qp], p);
        p += 1.0;
      }
      break;
  }

  // Loop Through Variables
  Real sumL = 0.0;
  for (unsigned int i = 1; i < _order; ++i)
    sumL += (*_vals[i])[_qp] * 0.5;

  val -= ((*_vals[0])[_qp] * sumL);

  return val;
}
