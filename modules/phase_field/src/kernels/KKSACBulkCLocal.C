//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSACBulkCLocal.h"

registerMooseObject("PhaseFieldApp", KKSACBulkCLocal);

InputParameters
KKSACBulkCLocalIntermediate::validParams()
{
  InputParameters params = KKSACBulkCBase::validParams();
  params.addClassDescription("KKS model kernel (part 2 of 2) for the Bulk Allen-Cahn. This "
                             "includes all terms dependent on chemical potential.");
  params.addRequiredParam<MaterialPropertyName>("ca", "a-phase concentration");
  params.addRequiredParam<MaterialPropertyName>("cb", "b-phase concentration");
  return params;
}

KKSACBulkCLocalIntermediate::KKSACBulkCLocalIntermediate(const InputParameters & parameters)
  : KKSACBulkCBase(parameters, getParam<MaterialPropertyName>("ca")),
    _ca(getMaterialProperty<Real>("ca")),
    _cb(getMaterialProperty<Real>("cb"))
{
  _ca_var = libMesh::invalid_uint;
  _cb_var = libMesh::invalid_uint;
}

template class KKSACBulkCBaseResidual<KKSACBulkCLocalIntermediate>;
