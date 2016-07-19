/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MinimizeProperty.h"

template<>
InputParameters validParams<MinimizeProperty>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<MaterialPropertyName>("prop", "Target property to minimize");
  params.addCoupledVar("args", "Vector of other arguments of the property");
  return params;
}

MinimizeProperty::MinimizeProperty(const InputParameters & parameters) :
    DerivativeMaterialInterface<JvarMapInterface<Kernel>>(parameters),
    _dPdu(getMaterialPropertyDerivative<Real>("prop", _var.name())),
    _d2Pdu2(getMaterialPropertyDerivative<Real>("prop", _var.name(), _var.name()))
{
  // Get number of coupled variables
  unsigned int nvar = _coupled_moose_vars.size();

  // Reserve space for derivatives
  _d2Pduarg.resize(nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < nvar; ++i)
    _d2Pduarg[i] = &getMaterialPropertyDerivative<Real>("prop", _var.name(), _coupled_moose_vars[i]->name());
}

void
MinimizeProperty::initialSetup()
{
  validateNonlinearCoupling<Real>("prop");
}

Real
MinimizeProperty::computeQpResidual()
{
  return std::pow(_dPdu[_qp], 2.0) * _test[_i][_qp];;
}

Real
MinimizeProperty::computeQpJacobian()
{
  return 2.0 * _dPdu[_qp] * _d2Pdu2[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}

Real
MinimizeProperty::computeQpOffDiagJacobian(unsigned int jvar)
{
  // Get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!mapJvarToCvar(jvar, cvar))
    return 0.0;

  // Set off-diagonal Jacobian term property derivatives
  return 2.0 * _dPdu[_qp] * (*_d2Pduarg[cvar])[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}
