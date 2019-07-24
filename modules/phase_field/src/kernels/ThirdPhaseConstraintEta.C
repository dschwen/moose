//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThirdPhaseConstraintEta.h"

registerMooseObject("PhaseFieldApp", ThirdPhaseConstraintEta);

template <>
InputParameters
validParams<ThirdPhaseConstraintEta>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription(
      "Lagrange multiplier kernel to constrain the phase field to the edges of the Gibbs simplex, "
      "disallowing the appearance of spurious third phases along grain boundaries.This kernel acts "
      "on a non-conserved order parameter eta_i.");
  params.addRequiredCoupledVar("etas",
                               "the other eta order parameters that are not the kernel variable.");
  params.addRequiredCoupledVar("lambda", "Lagrange multiplier");
  return params;
}

ThirdPhaseConstraintEta::ThirdPhaseConstraintEta(const InputParameters & parameters)
  : JvarMapKernelInterface<Kernel>(parameters),
    _n_eta(coupledComponents("etas")),
    _eta(_n_eta),
    _eta_map(getParameterJvarMap("eta")),
    _lambda(coupledValue("lambda")),
    _lambda_var(coupled("lambda"))
{
  // fetch order parameters
  for (unsigned int i = 0; i < _n_eta; ++i)
    _eta[i] = &coupledValue("etas", i);
}

Real
ThirdPhaseConstraintEta::etaProductSum()
{
  Real g = 0.0;
  for (unsigned int i = 0; i < _n_eta; ++i)
    for (unsigned int j = 0; j < i; ++j)
      g += (*_eta[i])[_qp] * (*_eta[j])[_qp];
  return g;
}

Real
ThirdPhaseConstraintEta::computeQpResidual()
{
  return _lambda[_qp] * etaProductSum() * _test[_i][_qp];
}

Real
ThirdPhaseConstraintEta::computeQpJacobian()
{
  return 0.0;
}

Real
ThirdPhaseConstraintEta::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _lambda_var)
    return _phi[_j][_qp] * etaProductSum() * _test[_i][_qp];

  // get the index into the eta parameter vector
  const auto eta = mapJvarToCvar(jvar, _eta_map);

  Real g = 0.0;
  for (unsigned int i = 0; i < _n_eta; ++i)
    if (i != eta)
      g += (*_eta[i])[_qp];

  return g * _phi[_j][_qp] * _test[_i][_qp];
}
