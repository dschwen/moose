//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DisplacedDiffusion.h"

registerMooseObject("MooseApp", DisplacedDiffusion);

template <>
InputParameters
validParams<DisplacedDiffusion>()
{
  InputParameters params = validParams<ALEKernel>();
  params.addClassDescription("The Laplacian operator ($-\\nabla \\cdot \\nabla u$), with the weak "
                             "form of $(\\nabla \\phi_i, \\nabla u_h)$.");
  return params;
}

DisplacedDiffusion::DisplacedDiffusion(const InputParameters & parameters) : ALEKernel(parameters)
{
  if (!getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh", "This kernel is only valid on the displaced mesh");
}

Real
DisplacedDiffusion::computeQpResidual()
{
  return _grad_u[_qp] * _grad_test[_i][_qp];
}

Real
DisplacedDiffusion::computeQpJacobian()
{
  return _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}

Real
DisplacedDiffusion::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  // off-diagonal Jacobian w.r.t. displacement variable
  if (_jvar_is_disp)
  {
    return _phiInvF * _grad_u[_qp] * _grad_test[_i][_qp] +
           _phiInvF * _grad_test_undisplaced[_i][_qp] * _grad_u[_qp];
  }

  return 0.0;
}
