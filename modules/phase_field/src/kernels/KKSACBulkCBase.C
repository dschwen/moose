//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSACBulkCBase.h"

InputParameters
KKSACBulkCBase::validParams()
{
  InputParameters params = KKSACBulkBase::validParams();
  params.addClassDescription("KKS model kernel (part 2 of 2) for the Bulk Allen-Cahn. This "
                             "includes all terms dependent on chemical potential.");
  return params;
}

KKSACBulkCBase::KKSACBulkCBase(const InputParameters & parameters, const std::string & ca_name)
  : KKSACBulkBase(parameters),
    _prop_dFadca(getMaterialPropertyDerivative<Real>("fa_name", ca_name)),
    _prop_d2Fadca2(getMaterialPropertyDerivative<Real>("fa_name", ca_name, ca_name)),
    _prop_d2Fadcadarg(_n_args)
{
  // get second partial derivatives wrt ca and other coupled variable
  for (unsigned int i = 0; i < _n_args; ++i)
    _prop_d2Fadcadarg[i] = &getMaterialPropertyDerivative<Real>("fa_name", ca_name, i);
}
