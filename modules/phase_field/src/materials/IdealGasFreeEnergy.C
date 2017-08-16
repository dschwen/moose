/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "IdealGasFreeEnergy.h"

template <>
InputParameters
validParams<IdealGasFreeEnergy>()
{
  InputParameters params = validParams<DerivativeParsedMaterialHelper>();
  params.addClassDescription("Free energy of an ideal gas.");
  params.addRequiredCoupledVar("T", "Temperature");
  params.addRequiredCoupledVar("c", "Concentration variable");
  params.addParam<Real>("m", "Gas atom mass");
  params.addParam<Real>("hbar", "Planck constant");
  params.addParam<Real>("kB", "Bolzman constant (default in [])");
  return params;
}

IdealGasFreeEnergy::IdealGasFreeEnergy(const InputParameters & parameters)
  : DerivativeParsedMaterialHelper(parameters), _T("T"), _c("c"), _hbar(0.0), _kB(0.0)
{
  EBFunction free_energy;
  // Definition of the free energy for the expression builder
  free_energy(_c) = 1.0 / 4.0 * (1.0 + _c) * (1.0 + _c) * (1.0 - _c) * (1.0 - _c);

  // Parse function for automatic differentiation
  functionParse(free_energy);
}
