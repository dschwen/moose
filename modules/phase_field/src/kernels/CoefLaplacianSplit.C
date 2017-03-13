/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CoefLaplacianSplit.h"

template <>
InputParameters
validParams<CoefLaplacianSplit>()
{
  InputParameters params = validParams<LaplacianSplit>();
  params.addClassDescription("Split with a variable that holds the Laplacian of a phase field "
                             "variable multiplied by a coefficient.");
  params.addParam<MaterialPropertyName>("coefficient", "Pre-factor coefficient on the Laplacian");
  return params;
}

CoefLaplacianSplit::CoefLaplacianSplit(const InputParameters & parameters)
  : LaplacianSplit(parameters), _coef(getMaterialProperty<Real>("coefficient"))
{
}

RealGradient
CoefLaplacianSplit::precomputeQpResidual()
{
  return _coef[_qp] * LaplacianSplit::precomputeQpResidual();
}

Real
CoefLaplacianSplit::computeQpOffDiagJacobian(unsigned int jvar)
{
  return _coef[_qp] * LaplacianSplit::computeQpOffDiagJacobian(jvar);
}
