/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "AllenCahnAniso.h"

template <>
InputParameters
validParams<AllenCahnAniso>()
{
  InputParameters params = AllenCahnBase<RealTensorValue>::validParams();
  params.addClassDescription("Allen-Cahn Kernel that uses a DerivativeMaterial Free Energy and "
                             "a tensor (anisotropic) mobility");
  return params;
}

AllenCahnAniso::AllenCahnAniso(const InputParameters & parameters)
  : AllenCahnBase<RealTensorValue>(parameters)
{
}
