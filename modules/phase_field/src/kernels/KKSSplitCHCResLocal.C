//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSSplitCHCResLocal.h"

registerMooseObject("PhaseFieldApp", KKSSplitCHCResLocal);

InputParameters
KKSSplitCHCResLocal::validParams()
{
  InputParameters params = KKSSplitCHCResBase::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "ca", "phase concentration corresponding to the non-linear variable of this kernel");
  return params;
}

KKSSplitCHCResLocal::KKSSplitCHCResLocal(const InputParameters & parameters)
  : KKSSplitCHCResBase(parameters, getParam<MaterialPropertyName>("ca"))
{
}
