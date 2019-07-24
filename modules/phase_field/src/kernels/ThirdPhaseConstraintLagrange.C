//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThirdPhaseConstraintLagrange.h"

registerMooseObject("PhaseFieldApp", ThirdPhaseConstraintLagrange);

template <>
InputParameters
validParams<ThirdPhaseConstraintLagrange>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription(
      "Lagrange multiplier kernel to constrain the phase field to the edges of the Gibbs simplex, "
      "disallowing the appearance of spurious third phases along grain boundaries.");
  params.addRequiredCoupledVar("etas", "eta_i order parameters, one for each h");
  params.addParam<Real>("epsilon", 1e-9, "Shift factor to avoid a zero pivot");
  return params;
}

ThirdPhaseConstraintLagrange::ThirdPhaseConstraintLagrange(const InputParameters & parameters)
  : JvarMapKernelInterface<Kernel>(parameters),
    _n_eta(coupledComponents("etas")),
    _eta(_n_eta),
    _eta_map(getParameterJvarMap("eta")),
    _epsilon(getParam<Real>("epsilon"))
{
  // fetch order parameters
  for (unsigned int i = 0; i < _n_eta; ++i)
    _eta[i] = &coupledValue("etas", i);
}

Real
ThirdPhaseConstraintLagrange::computeQpResidual()
{
  Real g = -_epsilon * _u[_qp];
  for (unsigned int i = 0; i < _n_eta; ++i)
    for (unsigned int j = 0; j < i; ++j)
      for (unsigned int k = 0; k < j; ++k)
        g += (*_eta[i])[_qp] * (*_eta[j])[_qp] * (*_eta[k])[_qp];

  return _test[_i][_qp] * g;
}

Real
ThirdPhaseConstraintLagrange::computeQpJacobian()
{
  return _test[_i][_qp] * -_epsilon * _phi[_j][_qp];
}

Real
ThirdPhaseConstraintLagrange::computeQpOffDiagJacobian(unsigned int jvar)
{
  const auto i = mapJvarToCvar(jvar, _eta_map);

  Real g = 0.0;
  for (unsigned int j = 0; j < _n_eta; ++j)
    if (j != i)
      for (unsigned int k = 0; k < j; ++k)
        g += _phi[_j][_qp] * (*_eta[j])[_qp] * (*_eta[k])[_qp];

  return g * _test[_i][_qp];
}
