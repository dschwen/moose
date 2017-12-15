/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "AllenCahn.h"

template <>
InputParameters
validParams<AllenCahn>()
{
  InputParameters params = AllenCahnBase<Real>::validParams();
  params.addClassDescription("Allen-Cahn Kernel that uses a DerivativeMaterial Free Energy and "
                             "a scalar (isotropic) mobility");
  return params;
}

AllenCahn::AllenCahn(const InputParameters & parameters) : AllenCahnBase<Real>(parameters) {}
