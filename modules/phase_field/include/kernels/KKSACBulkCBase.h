//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KKSACBulkBase.h"

// Forward Declarations

/**
 * KKSACBulkCBase child class for the phase concentration difference term
 * \f$ \frac{dh}{d\eta}\frac{dF_a}{dc_a}(c_a-c_b) \f$
 * in the the Allen-Cahn bulk residual.
 *
 * The non-linear variable for this Kernel is the order parameter 'eta'.
 */
class KKSACBulkCBase : public KKSACBulkBase
{
public:
  static InputParameters validParams();

  KKSACBulkCBase(const InputParameters & parameters, const std::string & ca_name);

protected:
  ///@{ variable numbers of the phase concentrations (or libMesh::invalid_uint if not applicable)
  unsigned int _ca_var;
  unsigned int _cb_var;
  ///@}

  /// Derivative of the free energy function \f$ \frac d{dc_a} F_a \f$
  const MaterialProperty<Real> & _prop_dFadca;

  /// Second derivative of the free energy function \f$ \frac {d^2}{dc_a^2} F_a \f$
  const MaterialProperty<Real> & _prop_d2Fadca2;

  /// Mixed partial derivatives of the free energy function wrt ca and
  /// any other coupled variables \f$ \frac {d^2}{dc_a dq} F_a \f$
  std::vector<const MaterialProperty<Real> *> _prop_d2Fadcadarg;
};

/**
 * KKSACBulkCBaseResidual implements the residual calculation of both classes
 * which are derived from KKSACBulkCBase. Those derived classes use coupled variables and
 * material properties for teh phase concentration respectively. KKSACBulkCBaseResidual
 * is templated on the derived class and through generic programming can be built
 * with _ca/_cb being either a VariableValue or a MaterialProperty<Real>.
 */
template <class T>
class KKSACBulkCBaseResidual : public T
{
public:
  static InputParameters validParams() { return T::validParams(); }

  KKSACBulkCBaseResidual(const InputParameters & parameters) : T(parameters) {}

protected:
  Real computeDFDOP(typename T::PFFunctionType type) override;
  Real computeQpOffDiagJacobian(unsigned int jvar) override;

  using T::_ca;
  using T::_cb;
  using T::_i;
  using T::_j;
  using T::_L;
  using T::_phi;
  using T::_prop_d2Fadca2;
  using T::_prop_d2Fadcadarg;
  using T::_prop_d2h;
  using T::_prop_dFadca;
  using T::_prop_dh;
  using T::_qp;
  using T::_test;
};

template <class T>
Real
KKSACBulkCBaseResidual<T>::computeDFDOP(typename T::PFFunctionType type)
{
  const Real A1 = _prop_dFadca[_qp] * (_ca[_qp] - _cb[_qp]);
  switch (type)
  {
    case T::Residual:
      return _prop_dh[_qp] * A1;

    case T::Jacobian:
      return _phi[_j][_qp] * _prop_d2h[_qp] * A1;
  }

  mooseError("Invalid type passed in");
}

template <class T>
Real
KKSACBulkCBaseResidual<T>::computeQpOffDiagJacobian(unsigned int jvar)
{
  // first get dependence of mobility _L on other variables using parent class
  // member function
  Real res = ACBulk<Real>::computeQpOffDiagJacobian(jvar);

  // Then add dependence of KKSACBulkF on other variables
  // Treat ca and cb specially, as they appear in the residual
  if (jvar == this->_ca_var)
    return res + _L[_qp] * _prop_dh[_qp] *
                     ((_ca[_qp] - _cb[_qp]) * _prop_d2Fadca2[_qp] + _prop_dFadca[_qp]) *
                     _phi[_j][_qp] * _test[_i][_qp];

  if (jvar == this->_cb_var)
    return res - _L[_qp] * _prop_dh[_qp] * _prop_dFadca[_qp] * _phi[_j][_qp] * _test[_i][_qp];

  //  for all other vars get the coupled variable jvar is referring to
  const unsigned int cvar = this->mapJvarToCvar(jvar);

  res += _L[_qp] * _prop_dh[_qp] * (*_prop_d2Fadcadarg[cvar])[_qp] * (_ca[_qp] - _cb[_qp]) *
         _phi[_j][_qp] * _test[_i][_qp];

  return res;
}
